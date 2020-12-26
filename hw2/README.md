## HW2 Requirement

Server 端程式的開發要能:
1. 提供 Client 端的註冊與登入
2. 發送 Client 目前的帳戶餘額與上線清單的回覆訊息給 Client
3. 接收處理 Client 端離線前的通知
* Server 提供的功能請使用 thread 及 worker pool 的方式進行程式的開發，不要使用 fork。

## Environment
* Ubuntu 20.04 LTS / Linux 5.4.0-56-generic
* g++ 9.3.0

## Execute
1. compile server on terminal
```
make
```

2. run server on terminal
```
./server
```

3. run `hw1/client` to communicate with this server
> [client execution and commands](../hw1/README.md)
