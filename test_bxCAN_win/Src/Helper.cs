using System;
using System.Collections.Generic;
using System.IO.Ports;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Text.RegularExpressions;
using System.Windows.Controls;

namespace testICAM
{
    public static class Helper
    { 
        public static string ToHex(this string message, bool toHex)
        {            
            if (toHex)
            {
                var res = new StringBuilder();
                byte[] bytes = Encoding.ASCII.GetBytes(message);

                foreach (byte b in bytes)
                    res.Append(string.Format("{0:X2} ", b));

                return res.ToString();
            } else
            {
                return message;
            }            
        }
        public static string ToStringHex(this byte[] ba, int size = 0)
        {
            int cnt = (size == 0 ? ba.Length : size);

            StringBuilder hex = new StringBuilder(cnt * 3);
            for (int i=0; i<cnt; i++)
                hex.AppendFormat("{0:x2} ", ba[i]);

            return hex.ToString();
        }

        public static bool ConnectTo(this SerialPort serialPort, string name, int speed, Parity parity)
        {
            if (string.IsNullOrWhiteSpace(name))
                throw new ArgumentException("Wrong port-name");

            serialPort.Close();
            serialPort.PortName = name;
            serialPort.BaudRate = speed;
            serialPort.Parity = parity;
            serialPort.WriteTimeout = 500;

            try
            {
                serialPort.Open();
            }
            catch (Exception e)
            {
                throw new Exception(e.Message);
            }
            return serialPort.IsOpen;
        }
        public static byte[] PutByte(this byte[] self, int index, byte value)
        {
            self[index] = value;
            return self;
        }
        public static byte[] PutUInt16(this byte[] self, int index, UInt16 value)
        {
            self[index] = (byte)(value >> 8);
            self[index+1] = (byte)(value);
            return self;
        }        

        public static byte[] SetBit(this byte[] self, int index, bool value)
        {
            int byteIndex = index / 8;
            int bitIndex = index % 8;
            byte mask = (byte)(1 << bitIndex);

            self[byteIndex] = (byte)(value ? (self[byteIndex] | mask) : (self[byteIndex] & ~mask));
            return self;
        }
        /**
         * установка бита indexBit в байте indexByte в массиве
         */
        public static byte[] SetBit(this byte[] self, int indexByte, int indexBit, bool value)
        {
            byte mask = (byte)(1 << indexBit);

            self[indexByte] = (byte)(value ? (self[indexByte] | mask) : (self[indexByte] & ~mask));
            return self;
        }
        public static byte[] SetBit(this byte[] self, int indexByte, int indexBit, bool? value)
        {
            byte mask = (byte)(1 << indexBit);

            self[indexByte] = (byte)(value == true ? (self[indexByte] | mask) : (self[indexByte] & ~mask));
            return self;
        }
        public static byte[] SetBit2(this byte[] self, int indexByte, int indexBit, int value)
        {
            bool value1 = (value & 1) > 0;
            bool value2 = (value & 2) > 0;

            byte mask = (byte)(1 << indexBit);
            self[indexByte] = (byte)(value1 ? (self[indexByte] | mask) : (self[indexByte] & ~mask));
                 mask = (byte)(1 << (indexBit+1));
            self[indexByte] = (byte)(value2 ? (self[indexByte] | mask) : (self[indexByte] & ~mask));

            return self;
        }

        public static byte[] ToggleBit(this byte[] self, int index)
        {
            int byteIndex = index / 8;
            int bitIndex = index % 8;
            byte mask = (byte)(1 << bitIndex);

            self[byteIndex] ^= mask;
            return self;
        }
        public static byte[] ToggleBit(this byte[] self, int indexByte, int indexBit)
        {
            byte mask = (byte)(1 << indexBit);

            self[indexByte] ^= mask;
            return self;
        }

        public static bool GetBit(this byte[] self, int index)
        {
            int byteIndex = index / 8;
            int bitIndex = index % 8;
            byte mask = (byte)(1 << bitIndex);

            return (self[byteIndex] & mask) != 0;
        }
        public static byte GetBit2(this byte[] self, int index)
        {
            byte res = 0;
            if (self.GetBit(index)) res += 1;
            if (self.GetBit(index+1)) res += 2;

            return res;
        }

        public static char GetBitChar(this byte[] self, int index)
        {
            int byteIndex = index / 8;
            int bitIndex = index % 8;
            byte mask = (byte)(1 << bitIndex);

            return ((self[byteIndex] & mask) != 0 ? '1' : '0');
        }

    }
}
