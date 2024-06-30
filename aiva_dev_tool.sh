#!/bin/sh

set +x

DEV=$1
PROJECT_PATH=$2
PROJECT_NAME=`grep "PROJ_NAME.*:=" $PROJECT_PATH/Makefile | awk '{print $3}'`
IMAGE_FILE=`find $PROJECT_PATH -type f -name "$PROJECT_NAME.img"`
echo UART Port: $DEV
echo Project: $PROJECT_NAME
echo Project Image: $IMAGE_FILE
echo ""


setup_uart_port() {
    stty -F $DEV 460800
}

kill_minicom() {
    SERVICE="minicom"
    if pgrep -x "$SERVICE" >/dev/null
    then
        echo "$SERVICE is running"
        killall -9 minicom
    else
        echo "$SERVICE stopped"
    fi
}

load_image_only() {
    echo -e "\e[34mrun command:\e[33m\n $0 $1 $2\e[0m"
    echo -e "\e[32mstart load image....\e[0m"
    echo "load part fw0" > $DEV
    sleep 0.5
    sz --ymodem $IMAGE_FILE > $DEV < $DEV
}

match_pattern_and_load() {
    echo -e "\e[34mrun command:\e[33m\n $0 $1 $2\e[0m"

    # match_pattern $DEV 'shell...'

    # echo -e "\e[32mPattern detected........ok\e[0m"

    # echo "" > $DEV
    # echo "" > $DEV

    # echo "load part fw0" > $DEV
    # sleep 0.5
    # sz --ymodem $IMAGE_FILE > $DEV < $DEV

    ./tools/aiva_uart_dl -vv --ymodem $IMAGE_FILE > $DEV < $DEV
}

rts_set_high_with_delay() {
    (sleep 0.5; ./tools/rts_clear $DEV) &
}

run_minicom() {
    rts_set_high_with_delay
    minicom -D $DEV -b 460800 -con -C minicom.cap
}

# Set the disable_ctrl_c function as the handler for SIGINT
trap disable_ctrl_c SIGINT

while true; do
    echo -e "\e[32mPlease select an option:\e[0m"
    echo "1. None"
    echo "2. Build project \"$PROJECT_NAME\""
    echo "3. Load image and run minicom"
    echo "4. Minicom only"
    echo "5. Build & Load & Debug"
    echo "6. Load xnn model file"
    echo "7. Load fw0 image by fw1 app"
    echo "q. Quit"

    read -p "Your choice: " choice
    echo ""

    case $choice in
        1)
            ;;
        2)
            make -C $PROJECT_PATH
            ;;
        3) 
            IMAGE_FILE=`find $PROJECT_PATH -type f -name "$PROJECT_NAME.img"`
            kill_minicom

            setup_uart_port $DEV

            rts_set_high_with_delay
            match_pattern_and_load $DEV $IMAGE_FILE

            run_minicom
            ;;
        4)
            run_minicom
            ;;
        5)
            make -C $PROJECT_PATH
            if [ ! $? -eq 0 ]; then
                continue
            fi

            IMAGE_FILE=`find $PROJECT_PATH -type f -name "$PROJECT_NAME.img"`
            echo  $IMAGE_FILE
            python tools/aiva_uart_dl.py -p fw0 -f $IMAGE_FILE -w -j 0x300e50a0

            echo "reboot" > $DEV

            run_minicom
            ;;
        6)
            IMAGE_FILE=`find $PROJECT_PATH -type f -name "*.xpack"`
            echo  $IMAGE_FILE
            python tools/aiva_uart_dl.py -p xnn -f $IMAGE_FILE -w -j 0x300e50a0
            ;;
        7)
            IMAGE_FILE=`find $PROJECT_PATH -type f -name "$PROJECT_NAME.img"`
            echo  $IMAGE_FILE
            python tools/aiva_uart_dl.py -p fw0 -f $IMAGE_FILE -w -j 0x300e50a0

            echo "reboot" > $DEV
            ;;
        q) exit ;;
        *) echo "Invalid choice. Please try again." ;;
    esac

    # read -n1 -p "Press any key to continue..." key
    # echo ""
done
