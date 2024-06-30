import os
import fcntl
import termios
import argparse

def set_rts(device, rts_level):
  """
  Sets the RTS pin of the specified device to the desired level (low or high).

  Args:
      device (str): The name of the serial device (e.g., "/dev/ttyUSB0").
      rts_level (str): The desired RTS level ("low" or "high").

  Returns:
      bool: True on success, False on failure.
  """

  # Validate RTS level input
  if rts_level not in ("low", "high"):
    print(f"Invalid RTS level '{rts_level}'. Please specify 'low' or 'high'.")
    return False

  # Open the serial port
  try:
    fd = os.open(device, os.O_RDWR | os.O_NOCTTY)
  except OSError as e:
    print(f"Failed to open serial port '{device}': {e}")
    return False

  # Get current control flags
  try:
    flags = termios.tcgetattr(fd)
  except termios.error as e:
    print(f"Error retrieving serial port settings: {e}")
    os.close(fd)
    return False

  # Set RTS level based on input
  if rts_level == "low":
    flags[0] &= ~termios.TIOCM_RTS
  elif rts_level == "high":
    # Attempt to set RTS high (might not work on all systems)
    flags[0] |= termios.TIOCM_RTS  # Uncomment this line if needed

  # Set the new control flags
  try:
    termios.tcsetattr(fd, termios.TCSANOW, flags)
  except termios.error as e:
    print(f"Error setting RTS level: {e}")
    os.close(fd)
    return False

  # Success message
  print(f"RTS pin set to '{rts_level}' on {device}")
  os.close(fd)
  return True

if __name__ == "__main__":
  parser = argparse.ArgumentParser(description="Set RTS pin on a serial device.")
  parser.add_argument("device", help="The name of the serial device (e.g., /dev/ttyUSB0)")
  parser.add_argument("rts_level", choices=["low", "high"], help="The desired RTS level (low or high)")
  args = parser.parse_args()

  set_rts(args.device, args.rts_level)
