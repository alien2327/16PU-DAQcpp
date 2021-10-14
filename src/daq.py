import socket, select
import datetime
import logging

_RBCP_VER = 255
_RBCP_CMD_WR = 128
_RBCP_CMD_RD = 192
_RBCP_PORT = 4660
_RBCP_OFFSET = 8
_TCP_PORT = 24

_OPTION_VALUE = 174760
_DATA_BUF_SIZE = 2048
_RD_BUF_SIZE = 40

_HEADER = [b"TRANSVERS", b'wave']
_FOOTER = [b"DATA processed with the 16-pu-monitor circuit", b'data']
_MODE = {0: "Process", 2: "Wave", 3: "Idle"}
_STATUS = {0: "Sample memory is neither full nor empty.",
            1: " Sample memory is full.",
            2: " Sample memory is empty."}

_IP_ADDR = "10.72.108.42"

class DAQ(object):
    def __init__(self) -> None:
        self.rbcp_id = 1
        self.trigger_on = False
        self.packet = None

        self.sockUDP = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, 0)
        self.sockUDP.connect((_IP_ADDR, _RBCP_PORT))
        self.sockUDP.settimeout(1)
        self.check()

        self.sockTCP = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
        self.sockTCP.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, _OPTION_VALUE)
        self.sockTCP.connect((_IP_ADDR,_TCP_PORT))

    def run(self):
        while(True):
            cmd_line = input("Enter Command: ").split()
            try:
                if cmd_line[0] == "set":
                    if cmd_line[1] == "gain":
                        if cmd_line[2:] == None:
                            print("Please Enter Value(s) for command")
                        else:
                            self._set_gain(cmd_line[2:])
                    elif cmd_line[1] == "mode":
                        if cmd_line[2:] == None:
                            print("Please Enter Value(s) for command")
                        else:
                            self._set_mode(cmd_line[2:][0])
                    elif cmd_line[1] == "number":
                        if cmd_line[2:] == None:
                            print("Please Enter Value(s) for command")
                        else:
                            self._set_data_num(cmd_line[2:][0])
                    elif cmd_line[1] == "trigger":
                        if cmd_line[2:] == None:
                            print("Please Enter Value(s) for command")
                        else:
                            self._set_trigger_on()
                    elif cmd_line[1] == "delay":
                        if cmd_line[2:] == None:
                            print("Please Enter Value(s) for command")
                        else:
                            self._set_delay_clock(cmd_line[2:][0])
                    else:
                        print("Invalid command")
                elif cmd_line[0] == "get":
                    if cmd_line[1] == "data":
                        if cmd_line[2:] == None:
                            print("Please Enter Value(s) for command")
                        else:
                            self._get_data(cmd_line[2:][0]) 
                    elif cmd_line[1] == "gain":
                        if cmd_line[2:] == None:
                            print("Please Enter Value(s) for command")
                        else:
                            self._get_gain()
                    elif cmd_line[1] == "mode":
                        if cmd_line[2:] == None:
                            print("Please Enter Value(s) for command")
                        else:
                            self._get_mode()
                    elif cmd_line[1] == "status":
                        if cmd_line[2:] == None:
                            print("Please Enter Value(s) for command")
                        else:
                            self._get_mem_status()
                    elif cmd_line[1] == "number":
                        if cmd_line[2:] == None:
                            print("Please Enter Value(s) for command")
                        else:
                            self._get_data_num()
                    elif cmd_line[1] == "trigger":
                        if cmd_line[2:] == None:
                            print("Please Enter Value(s) for command")
                        else:
                            self._get_trigger_on()
                    else:
                        print("Invalid command")
                elif cmd_line[0] == "reset":
                    self._reset()
                elif cmd_line[0] == "exit":
                    print("DAQ Script Terminated")
                    self.terminate()
                    break
                else:
                    print("Invalid command")
            except IndexError:
                print("Invalid command")
            print(self.packet)
        return

    def terminate(self):
        self.sockUDP.close()
        self.sockTCP.close()
        return

    def check(self):
        self.packet = [_RBCP_VER, _RBCP_CMD_RD, self.rbcp_id, _RD_BUF_SIZE, 0, 0, 0, 0]
        while True: 
            self.sockUDP.send(bytearray(self.packet))
            a = select.select([self.sockUDP],[],[], 1)
            if not a[0]:
                print( "*** Timeout! ***")
                self.packet[2] += 1
                if (self.packet[2] - self.rbcp_id) < 3:
                    self.rbcp_id = self.packet[2]
                    return
            else:
                break
        print("Succeess to connect!")

    def _set_gain(self, gain: list):
        if len(gain) > 16:
            print("Invalid electrode index")
            return
        for ch, g in enumerate(gain):
            self.packet = [_RBCP_VER, _RBCP_CMD_WR, self.rbcp_id, 2, 0, 0, 0, ch * 2 + 1,
                    (int(32768 * g) & 0xFF00) >> 8, int(32768 * g) & 0xFF]
            self.sockUDP.send(bytearray(self.packet))
            recvData, ipAddr = self.sockUDP.recvfrom(_DATA_BUF_SIZE)
            self.rbcp_id += 1
        return
    
    def _set_mode(self, mode):
        self.packet = [_RBCP_VER, _RBCP_CMD_WR, self.rbcp_id, 1, 0, 0, 0, 0, 0xFF & int(mode)]
        self.sockUDP.send(bytearray(self.packet))
        recvData, ipAddr = self.sockUDP.recvfrom(_DATA_BUF_SIZE)
        self.rbcp_id += 1
        return

    def _set_data_num(self, num):
        num = int(num)
        self.packet = [_RBCP_VER, _RBCP_CMD_WR, self.rbcp_id, 3, 0, 0, 0, 33, 
                (0x00FF0000 & num)>>16, (0x0000FF00 & num)>>8, 0x000000FF & num]
        self.sockUDP.send(bytearray(self.packet))
        recvData, ipAddr = self.sockUDP.recvfrom(_DATA_BUF_SIZE)
        self.rbcp_id += 1
        return

    def _set_trigger_on(self):
        self.trigger_on = not self.trigger_on
        switch = "ON" if self.trigger_on else "OFF"
        print("Test trigger mode ", switch)
        self.packet = [_RBCP_VER, _RBCP_CMD_WR, self.rbcp_id, 1, 0, 0, 0, 37, self.trigger_on]
        self.sockUDP.send(bytearray(self.packet))
        recvData, ipAddr = self.sockUDP.recvfrom(_DATA_BUF_SIZE)
        self.rbcp_id += 1
        return

    def _set_delay_clock(self, delay):
        self.packet = [_RBCP_VER, _RBCP_CMD_WR, self.rbcp_id, 1, 0, 0, 0, 38, 0x3F & int(delay)]
        self.sockUDP.send(bytearray(self.packet))
        recvData, ipAddr = self.sockUDP.recvfrom(_DATA_BUF_SIZE)
        self.rbcp_id += 1
        return
    
    def _get_gain(self):
        self.packet = [_RBCP_VER, _RBCP_CMD_RD, self.rbcp_id, _RD_BUF_SIZE, 0, 0, 0, 0]
        self.sockUDP.send(bytearray(self.packet))
        recvData, ipAddr = self.sockUDP.recvfrom(_DATA_BUF_SIZE)
        gain = recvData[_RBCP_OFFSET + 1:_RBCP_OFFSET + 17]
        print("Recieved gain data from ", ipAddr)
        print("Gain: ", gain)
        return gain

    def _get_data(self, shots):
        print('Start recieving data')

        for s in range(shots):
            print("Shot num #", s)
            for ch in range(16):
                self.packet = [_RBCP_VER, _RBCP_CMD_WR, self.rbcp_id, 1, 0, 0, 0, 36, ch]
                self.sockUDP.send(bytearray(self.packet))
                recvData, ipAddr = self.sockUDP.recvfrom(_DATA_BUF_SIZE)
                recvData = self.sockTCP.recv(_DATA_BUF_SIZE)
                while True:
                    recvData += self.sockTCP.recv(_DATA_BUF_SIZE)
                    if _FOOTER[1] in recvData:
                        index = recvData.find(_FOOTER[1])                    
                        print(recvData[_RD_BUF_SIZE:index + len(_FOOTER[1])])
                        break
                    else: 
                        print(recvData[_RD_BUF_SIZE:])
                    recvData = recvData[len(recvData) - _RD_BUF_SIZE:]
                self.rbcp_id += 1

        print('End recieving data')
        return

    def _get_mode(self):
        self.packet = [_RBCP_VER, _RBCP_CMD_RD, self.rbcp_id, _RD_BUF_SIZE, 0, 0, 0, 0]
        self.sockUDP.send(bytearray(self.packet))
        recvData, ipAddr = self.sockUDP.recvfrom(_DATA_BUF_SIZE)
        mode = recvData[_RBCP_OFFSET + 0]
        print("Recieved mode information from ", ipAddr)
        print("Current Mode: ", _MODE[mode])
        return mode

    def _get_data_num(self):
        self.packet = [_RBCP_VER, _RBCP_CMD_RD, self.rbcp_id, _RD_BUF_SIZE, 0, 0, 0, 0]
        self.sockUDP.send(bytearray(self.packet))
        recvData, ipAddr = self.sockUDP.recvfrom(_DATA_BUF_SIZE)
        data_num = int(recvData[_RBCP_OFFSET + 33]) * 65536 + int(recvData[_RBCP_OFFSET + 34]) * 256 + int(recvData[_RBCP_OFFSET + 35])
        print("Recieved data number from ", ipAddr)
        print("Current data number: ", data_num)
        return data_num

    def _get_mem_status(self):
        self.packet = [_RBCP_VER, _RBCP_CMD_RD, self.rbcp_id, _RD_BUF_SIZE, 0, 0, 0, 0]
        self.sockUDP.send(bytearray(self.packet))
        recvData, ipAddr = self.sockUDP.recvfrom(_DATA_BUF_SIZE)
        status = recvData[_RBCP_OFFSET + 37]
        print("Recieved current memory status from ", ipAddr)
        print("Current status: ", _STATUS[status])
        return status

    def _get_trigger_on(self):
        self.packet = [_RBCP_VER, _RBCP_CMD_RD, self.rbcp_id, _RD_BUF_SIZE, 0, 0, 0, 0]
        self.sockUDP.send(bytearray(self.packet))
        recvData, ipAddr = self.sockUDP.recvfrom(_DATA_BUF_SIZE)
        trigger = recvData[_RBCP_OFFSET + 38]
        switch = "ON" if trigger else "OFF"
        print("Recieved current trigger mode from ", ipAddr)
        print("Current trigger mode: ", switch)
        return trigger

    def _reset(self):
        self.packet = [_RBCP_VER, _RBCP_CMD_RD, self.rbcp_id, 1, 0xFF, 0xFF, 0xFF, 0x10, 0x00]
        self.sockUDP.send(bytearray(self.packet))
        recvData, ipAddr = self.sockUDP.recvfrom(_DATA_BUF_SIZE)
        self.rbcp_id += 1
        return
    
def main():
    daq = DAQ()
    daq.run()

if __name__ == "__main__":
    main()
