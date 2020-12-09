## HW1 Requirement
本階段為 Client 端程式的開發，同學所撰寫的 Client 端程式必須要能：
a. 向 Server 註冊（註冊時輸入名稱以及初始存款金額）。
b. 登入助教所提供 Server 端程式（填入使用者名稱、Port Number）。
c. 向 Server 要最新的帳戶餘額與上線清單
d. 接收 Server 端回覆的 Client 目前的帳戶餘額與上線清單訊息。
e. 和其它 Client 傳輸交易訊息。
f. 進行離線動作前需主動告知 Server 端程式。

## Environment
* Ubuntu 20.04 LTS / Linux 5.4.0-56-generic
* g++ 9.3.0

## Execute
1. run server on terminal
```
./server
```

2. run client on terminal
```
g++ -o client client.cpp
./client
```

3. client commands
```
// register user accounts with deposit number
REGISTER#aaa#200
REGISTER#bbb#400

// login to account `aaa` at port 5000
aaa#5000

// list account balance and online account info
List

// Close socket connect
Exit
```
