* mipi 相关接口介绍
* mipi 相关接口比较底层，并且有 videoin 模块作为封装，是否有必要暴露给终端用户？
  1. 如果不暴露 mipi 相关底层接口给用户，需要将 videoin 模块文档写好
  2. videoin 模块，是否足以覆盖用户场景，或用户的不同需求？
  3. 用户的合理需求是否会导致频繁改动 sdk 来支持？

需要 video in porting guide