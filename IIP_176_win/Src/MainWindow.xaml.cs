using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.IO.Ports;
using System.Net;
using System.Net.Sockets;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace testWin
{
    /// <summary>
    /// Логика взаимодействия для MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        static System.Windows.Threading.DispatcherTimer timerSend;

        [Flags]
        enum LogFlags { None = 0, noReturn = 1, toFile = 2, noTime = 4 };

        static SerialPort serialPort;

        private bool isConnected;

        string logFileName;

        const int timerPeriod_ms = 300; //


        public MainWindow()
        {
            InitializeComponent();

            logFileName = @"./log.txt";

            string[] ports = SerialPort.GetPortNames();
            cbSerialPort.ItemsSource = ports;
            cbSerialPort.SelectedIndex = 0;

            serialPort = new SerialPort();

            timerSend = new System.Windows.Threading.DispatcherTimer();
            timerSend.Tick += DispatcherTimerSend_Tick;
            timerSend.Interval = TimeSpan.FromMilliseconds(timerPeriod_ms);
            timerSend.Start();

            LogAdd("Start");
        }

        private void btnDisconnect_Click(object sender, RoutedEventArgs e)
        {
            bool bMsg = false;

            if (serialPort != null)
                if (serialPort.IsOpen) {
                    bMsg = true;
                    serialPort.Close();
                    serialPort.DataReceived -= new SerialDataReceivedEventHandler(DataReceivedHandler);
                }

            btnConnect.IsEnabled = true;
            if (bMsg) LogAdd("Disconnect");
        }

        private void btnConnect_Click(object sender, RoutedEventArgs ea)
        {
            isConnected = false;
            btnConnect.IsEnabled = false;


            string name = cbSerialPort.Text;
            int speed = int.Parse(cbSpeed.Text);
            Parity parity = (Parity)Enum.Parse(typeof(Parity), cbParity.Text);

            LogAdd($"Connect: {name}:{speed}, {parity} - ", LogFlags.noReturn);

            try
            {
                isConnected = serialPort.ConnectTo(name, speed, parity);
            }
            catch (Exception e)
            {
                LogAdd(e.Message, LogFlags.noTime);
            }

            //если подключен последовательный порт, то кнопка "подключить" остается не доступна
            if (!(serialPort.IsOpen)) {
                btnConnect.IsEnabled = true;
                //начинаем слушать событие
            } else
            {
                try
                {
                    serialPort.DataReceived += new SerialDataReceivedEventHandler(DataReceivedHandler);
                }
                catch (Exception e)
                {
                    LogAdd(e.Message, LogFlags.noTime);
                }
            }

            Mouse.OverrideCursor = null;
            if (isConnected) LogAdd("Ok", LogFlags.noTime);
        }

        static readonly object writeLock = new object();

        private void LogAdd(string message, LogFlags flags = LogFlags.None)
        {
            if (fMain.IsInitialized == false) return;

            //add time
            if ((chkLogTime.IsChecked.Value == true) && (flags.HasFlag(LogFlags.noTime) == false))
                message = DateTime.Now.ToString("yyyy-MM-dd HH\\:mm\\:ss.fff") + "> " + message;

            //new line
            if (!flags.HasFlag(LogFlags.noReturn)) message += Environment.NewLine;
            
            //write to file
            if (!flags.HasFlag(LogFlags.toFile))
                lock (writeLock)
                    try
                    {
                        File.AppendAllText(logFileName, message);
                    }
                    catch (Exception e)
                    {
                        message += "[writeFileError]"+e.Message;
                    }
            
            txtLog.AppendText(message);
            txtLog.ScrollToEnd();
        }

        private void btnLogClear_Click(object sender, RoutedEventArgs e)
        {
            txtLog.Clear();
            //txtLog.Document.Blocks.Clear();
        }

        void DataReceivedHandler(object sender, SerialDataReceivedEventArgs ea)
        {
            Thread.Sleep(100); //wait all symbols            
            SerialPort port = (SerialPort)sender;

            int bytes = port.BytesToRead;

            if (bytes < 1) return;

            byte[] bfr0 = new byte[bytes];
            port.Read(bfr0, 0, bytes);
            
            // process data on the GUI thread
            try
            {
                Application.Current.Dispatcher.Invoke(new Action(() =>
                    LogAdd("Recv: " + bfr0.ToStringHex())
                ));
            }
            catch (Exception)
            {

            }
        }

        private void btnSerialRefresh_Click(object sender, RoutedEventArgs e)
        {
            string[] ports = SerialPort.GetPortNames();
            cbSerialPort.ItemsSource = null;
            cbSerialPort.Items.Clear();
            cbSerialPort.ItemsSource = ports;
        }

        private void ResetValue_Click(object sender, RoutedEventArgs e)
        {
            txtValue.Text = "0";
        }

        private void ResetValue2_Click(object sender, RoutedEventArgs e)
        {
            txtValue2.Text = "0";
        }

        private void SendDataUART(byte addr, int size, byte[] bfr)
        {            
            var bfrOut = new byte[size + 3];
            //addrH, addrL, len, b1, ..., b8
            bfrOut[0] = addr;
            for (int i = 0; i < size; i++)
                bfrOut[i + 1] = bfr[i];
            UInt16 crc = Helper.Crc16(bfrOut, 5);
            bfrOut[5] = (byte)(crc >> 8);
            bfrOut[6] = (byte)(crc);

            LogAdd("Send: " + bfrOut.ToStringHex());

            try
            {
                if (!serialPort.IsOpen)
                    btnConnect_Click(null, null);
                
                serialPort.Write(bfrOut, 0, bfrOut.Length);                
            }
            catch (Exception e)
            {
                LogAdd("Send: |" + e.GetType().ToString() + "| " + e.Message); ;
            }
        }

        private void fMain_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {

        }

        private void SendCmd_Click(object sender, RoutedEventArgs e)
        {
            var bfr = new byte[4];
            bool res;
            short value, value2;
            res = short.TryParse(txtValue.Text, out value);
            res = short.TryParse(txtValue2.Text, out value2);

            bfr[0] = (byte)(value);
            bfr[1] = (byte)(value >> 8);
            bfr[2] = (byte)(value2 & 0x7F);

            bfr.SetBit(2, 7, (cbBlinking.SelectedIndex & 1) > 0); //b23
            bfr.SetBit(3, 1, (cbBlinking.SelectedIndex & 4) > 0); //b24
            bfr.SetBit(3, 0, (cbBlinking.SelectedIndex & 2) > 0); //b25

            bfr.SetBit(3, 2, (cbColorLed.SelectedIndex & 1) > 0);
            bfr.SetBit(3, 3, (cbColorLed.SelectedIndex & 2) > 0);

            bfr.SetBit(3, 4, (cbPointPos.SelectedIndex & 1) > 0);
            bfr.SetBit(3, 5, (cbPointPos.SelectedIndex & 2) > 0);

            bfr.SetBit(3, 6, (cbBright.SelectedIndex & 1) > 0);
            bfr.SetBit(3, 7, (cbBright.SelectedIndex & 2) > 0);

            SendDataUART(GetAddrDevice(), 4, bfr);
        }

        int value128=0, dir128=1;
        private void DispatcherTimerSend_Tick(object sender, EventArgs e)
        {
            timerSend.Interval = TimeSpan.FromMilliseconds( (chkAutoInc.IsChecked == true) ? 90 : 333 );
            if (chkAutoInc.IsChecked == true)
            {
                value128 += dir128;
                if (value128 >= 127) dir128 = -1;
                if (value128 <= 0) dir128 = 1;

                if (chkSendShort.IsChecked == true)
                {
                    if (value128 < 125)
                    {
                        value128 = 125;
                        dir128 = 1;
                    }
                    if (value128 > 126)
                    {
                        value128 = 126;
                        dir128 = -1;
                    }
                }
                
                txtValue.Text = value128.ToString();
                txtValue2.Text = value128.ToString();
            }

            if (chkSendAuto.IsChecked == true) SendCmd_Click(null, null);
        }

        private byte GetAddrDevice()
        {
            byte addr = byte.Parse(txtAddr.Text);

            return addr;
        }
    }
}
