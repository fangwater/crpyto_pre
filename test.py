import websocket
import json
import threading

message_count = 0

def on_message(ws, message):
    global message_count
    # 增加消息计数，不打印消息
    message_count += 1  
    print(message)

def on_error(ws, error):
    print(error)

def on_close(ws, close_status_code, close_msg):
    print("### closed ###")

def on_open(ws):
    # Send the subscription message for @trade stream
    subscribe_message = json.dumps({
        "method": "SUBSCRIBE",
        "params": [
            "btcusdt@depth@100ms",
            "ethusdt@depth@100ms",
            "bnbusdt@depth@100ms"
        ],
        "id": 123
    })
    ws.send(subscribe_message)

def print_message_count():
    global message_count
    while True:
        print(f"Messages received: {message_count}")
        threading.Event().wait(1)  # 每隔一秒打印一次

if __name__ == "__main__":
    websocket.enableTrace(True)
    ws = websocket.WebSocketApp("wss://vps.mgsd.cc:8090/ws",
                                on_open=on_open,
                                on_message=on_message,
                                on_error=on_error,
                                on_close=on_close)

    # 创建并启动后台线程用于打印消息计数
    threading.Thread(target=print_message_count, daemon=True).start()

    ws.run_forever()
