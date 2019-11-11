# ws_im
基于Linux环境，c++实现的支持websocket协议的消息服务
# 说明:
1) 简单的c++ websocket网络框架。支持websocket协议握手和数据传输，可正常解包。数据协议格式参照以下链接：
https://www.cnblogs.com/chyingp/p/websocket-deep-in.html；
2）网络IO模型为基于非阻塞的IO复用，单线程环境下可正常收发数据，qps并不高，后期可根据需要优化IO
模型，如使用线程池+任务队列的形式实现高性能的并发；
3) CFrameworkService等单例模式不是线程安全的；
4) 暂时没用到BOOST/SGI STL智能指针；
5) 暂时没用到任何其他payload协议，如xml/json/protobuff等，只是用纯文本模拟消息转发；
6) 每个独立的模块实现文件都在同一目录；

   由于时间关系，上述暂时不规范或不安全的用法，后期会及时commit
# 功能
1) epoll IO复用的网络框架；
2) 支持websocket的握手、应用包解包;
3) 多client可连；
3) 消息转发;

# 开发环境
  VS2015(不支持跨平台编译构建，感兴趣可用VS2017 + SSH + linux跨平台开发调试)
# 运行环境
  ws_server（ws IM服务）、ws_client(压力测试client工具) 请移步Linux环境编译构建，方法见下方。
  
# 构建
ws IM服务：
cd ws_server
g++ -std=c++0x -o wss *.h *.cpp

ws 压测工具:
g++ -o wsc *.h *.cpp

# 运行

ws IM服务：

./wss ip  port &
-- ip:   服务地址(必填)
-- port: 服务端口(必填)

ws 压测工具:

./wsc  ip port n_clients
  
-- ip:          服务地址(必填)
-- port:        服务端口(必填)
-- n_clients:   并发client数量(必填)
  


