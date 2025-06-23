import os
import serial
import argparse

BAUD_RATE = 115200
TIMEOUT = 5

def send_img(ser, kernel_path):
    print("Sending kernel image size...")

    # 取得檔案大小
    kernel_size = os.stat(kernel_path).st_size
    ser.write((str(kernel_size) + "\n").encode())

    # 等待 bootloader 準備訊息
    msg = ser.read_until(b"Ready\n").decode()
    print(msg, end="")

    # 傳送整個 image
    print("Sending kernel binary...")
    with open(kernel_path, "rb") as image:
        data = image.read()
        ser.write(data)

    print("Done.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Send kernel image over UART")
    parser.add_argument("device", help="Serial device path (e.g., /dev/ttyUSB0)")

    args = parser.parse_args()

    ser = serial.Serial(args.device, BAUD_RATE, timeout=TIMEOUT)
    send_img(ser, "../kernel8.img")
    ser.close()