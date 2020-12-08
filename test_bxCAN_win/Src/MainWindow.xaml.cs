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

namespace testICAM
{
    /// <summary>
    /// Логика взаимодействия для MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        static System.Windows.Threading.DispatcherTimer timerSend;

        static int periodSendig = 5;

        [Flags]
        enum LogFlags { None = 0, noReturn = 1, toFile = 2, noTime = 4 };

        static SerialPort serialPort;

        private bool isConnected;

        string logFileName;


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
            timerSend.Interval = new TimeSpan(0, 0, periodSendig);
            timerSend.Start();

            var listForComboBox = new List<string> { "-", "Пожар", "АПС", "Аврал" };
            foreach (UIElement element in grid4xx.Children)
            {
                if (element is ComboBox)
                {
                    ComboBox cb = (ComboBox)element;
                    cb.ItemsSource = listForComboBox;
                    cb.SelectedIndex = 0;
                }
            }

            string msg = "Управление работает только при отключенных установках команды 4хх";
            chkFlasher1.ToolTip = msg;
            chkFlasher2.ToolTip = msg;
            chkFlasher3.ToolTip = msg;
            cbBuzzer.ToolTip = msg;

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
            string sCmd = "";

            int bytes = port.BytesToRead;

            if (bytes < 1) return;

            byte[] bfr0 = new byte[bytes];
            port.Read(bfr0, 0, bytes);
            
            byte[] bfr = new byte[8];
            for (int i = 0; i < (bytes - 3) && i < 8; i++) 
                bfr[i] = bfr0[i + 3];

            if (bfr0[0] == 1)
            {
                //Standard ID: 0x181       DLC: 6  Data: 0x04 0xF8 0x3C 0x00 0x00 0x20
                //Recv: 01 3F 08 04 3F 3C 00 00 20 00 00
                sCmd += "  Cmd=" + bfr0[0] + "  Addr=" + (bfr0[1] & 0x7F).ToString("X2");

                // 0 (смещение в пакете)
                sCmd += "  FG=" + (bfr[0]).ToString("X2");

                // 1
                sCmd += "\n\n  btn_pwr=[ " + bfr.GetBitChar(1 * 8 + 0) + bfr.GetBitChar(1 * 8 + 1) + bfr.GetBitChar(1 * 8 + 2) + " ]";
                sCmd += ", pwr=[ " + bfr.GetBitChar(1 * 8 + 3) + bfr.GetBitChar(1 * 8 + 4) + " ]";
                sCmd += ", err=" + bfr.GetBitChar(1 * 8 + 5) + bfr.GetBitChar(1 * 8 + 6) + bfr.GetBitChar(1 * 8 + 7);

                // 2
                string[] aBs = { " --- ", "Short", "Long ", "Fixed" };
                sCmd += "\n\nbtn_state=[ " + aBs[bfr[2] & 0x3] + ", " + aBs[(bfr[2] >> 2) & 0x3] + ", " + aBs[(bfr[2] >> 4) & 0x3] + " ]";

                // 3+4
                sCmd += "\n\nled_state=[";
                for (int i = 0; i < 11; i++)
                    sCmd += " " + bfr.GetBitChar(3 * 8 + i);

                // 4+
                sCmd += " ]\n\n  Flasher=["; //биты 3, 4, 5
                for (int i = 0; i < 3; i++)
                    sCmd += " " + bfr.GetBitChar(4 * 8 + 3 + i);
                //биты 6,7
                string[] aBz = { " --- ", "Пожар", " АПС ", "Аврал" };
                sCmd += " ], Buzzer=" + aBz[bfr.GetBit2(4 * 8 + 6)];

                // 5
                sCmd += "\n\n di_state=[ " + aBs[bfr.GetBit2(5 * 8)] + ", " + aBs[bfr.GetBit2(5 * 8 + 2)];
                string[] aBL = { " --- ", " вкл ", "редко", "часто" };
                sCmd += " ]\n\n  btn_led=" + aBL[bfr.GetBit2(5 * 8 + 4)] + " " + aBL[bfr.GetBit2(5 * 8 + 6)];
            }

            // process data on the GUI thread
            try
            {
                Application.Current.Dispatcher.Invoke(new Action(() =>
                    LogAdd("Recv: " + bfr0.ToStringHex())
                ));
                Application.Current.Dispatcher.Invoke(new Action(() =>
                    txtLog2.Text = bfr0.ToStringHex().Insert(8, " (") + ")\n\n" + sCmd
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

        private void SendDataUART(Int16 addr, int size, byte[] bfr)
        {
            txtSendBfr.Text = "Addr:0x" + addr.ToString("X3") + "  Size:" + size + "  Bfr:" + bfr.ToStringHex(size);

            var bfrOut = new byte[size + 3];
            //addrH, addrL, len, b1, ..., b8
            bfrOut[0] = (byte)(addr >> 8);
            bfrOut[1] = (byte)(addr & 0xFF);
            bfrOut[2] = (byte)(size);
            for (int i = 0; i < size; i++)
                bfrOut[i + 3] = bfr[i];

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

        private void Send2xx_Click(object sender, RoutedEventArgs e)
        {
            var bfr = new byte[8];
            bfr.SetBit(0, 0, chkLED1.IsChecked == true);
            bfr.SetBit(0, 1, chkLED2.IsChecked == true);
            bfr.SetBit(0, 2, chkLED3.IsChecked == true);
            bfr.SetBit(0, 3, chkLED4.IsChecked == true);
            bfr.SetBit(0, 4, chkLED5.IsChecked == true);
            bfr.SetBit(0, 5, chkLED6.IsChecked == true);
            bfr.SetBit(0, 6, chkLED7.IsChecked == true);
            bfr.SetBit(0, 7, chkLED8.IsChecked == true);
            bfr.SetBit(1, 0, chkLED9.IsChecked == true);
            bfr.SetBit(1, 1, chkLED10.IsChecked == true);
            bfr.SetBit(1, 2, chkLED11.IsChecked == true);

            bfr.SetBit(2, 0, chkFlasher1.IsChecked == true);
            bfr.SetBit(2, 1, chkFlasher2.IsChecked == true);
            bfr.SetBit(2, 2, chkFlasher3.IsChecked == true);

            bfr.SetBit(3, 0, (cbBuzzer.SelectedIndex & 1) > 0);
            bfr.SetBit(3, 1, (cbBuzzer.SelectedIndex & 2) > 0);

            bfr.SetBit(4, 0, (cbBtnLed1.SelectedIndex & 1) > 0);
            bfr.SetBit(4, 1, (cbBtnLed1.SelectedIndex & 2) > 0);

            bfr.SetBit(4, 2, (cbBtnLed2.SelectedIndex & 1) > 0);
            bfr.SetBit(4, 3, (cbBtnLed2.SelectedIndex & 2) > 0);

            byte fg = byte.Parse(txtFGroup.Text, System.Globalization.NumberStyles.HexNumber);

            SendDataUART((short)(0x200 + fg) , 5, bfr);
        }

        private void Send5xx_Click(object sender, RoutedEventArgs e)
        {
            var bfr = new byte[8];
            bfr.PutUInt16(0, UInt16.Parse(TxtBtnLedOff_Slow.Text));
            bfr.PutUInt16(2, UInt16.Parse(TxtBtnLedOn_Slow.Text));
            bfr.PutUInt16(4, UInt16.Parse(TxtBtnLedOff_Quick.Text));
            bfr.PutUInt16(6, UInt16.Parse(TxtBtnLedOn_Quick.Text));

            byte fg = byte.Parse(txtFGroup.Text, System.Globalization.NumberStyles.HexNumber);

            SendDataUART((short)(0x500 + fg), 8, bfr);
        }

        private void Send6xx_Click(object sender, RoutedEventArgs e)
        {
            var bfr = new byte[8];
            bfr.PutUInt16(0, UInt16.Parse(TxtPressTime_Short.Text));
            bfr.PutUInt16(2, UInt16.Parse(TxtPressTime_Long.Text));
            bfr.PutByte(4, byte.Parse(TxtPressTime_Fixed.Text));
            bfr.PutByte(5, byte.Parse(TxtPressTime_Save.Text));

            byte fg = byte.Parse(txtFGroup.Text, System.Globalization.NumberStyles.HexNumber);

            SendDataUART((short)(0x600 + fg), 6, bfr);
        }

        private void Send7xx_Click(object sender, RoutedEventArgs e)
        {
            var bfr = new byte[8];
            bfr.PutUInt16(0, UInt16.Parse(TxtCANsendingPeriod.Text));
            bfr.PutUInt16(2, UInt16.Parse(TxtBuzzerSilentTime.Text));
            bfr.PutUInt16(4, UInt16.Parse(TxtBtnFilter.Text));
            bfr.PutUInt16(6, UInt16.Parse(TxtBtnDILong.Text));

            byte fg = byte.Parse(txtFGroup.Text, System.Globalization.NumberStyles.HexNumber);

            SendDataUART((short)(0x700 + fg), 8, bfr);
        }

        private void Send4xx_Click(object sender, RoutedEventArgs e)
        {
            var bfr = new byte[8];

            bfr.SetBit(0, 0, chkLEDFlasher1.IsChecked);
            bfr.SetBit(0, 4, chkLEDFlasher2.IsChecked);
            bfr.SetBit(1, 0, chkLEDFlasher3.IsChecked);
            bfr.SetBit(1, 4, chkLEDFlasher4.IsChecked);
            bfr.SetBit(2, 0, chkLEDFlasher5.IsChecked);
            bfr.SetBit(2, 4, chkLEDFlasher6.IsChecked);
            bfr.SetBit(3, 0, chkLEDFlasher7.IsChecked);
            bfr.SetBit(3, 4, chkLEDFlasher8.IsChecked);
            bfr.SetBit(4, 0, chkLEDFlasher9.IsChecked);
            bfr.SetBit(4, 4, chkLEDFlasher10.IsChecked);
            bfr.SetBit(5, 0, chkLEDFlasher11.IsChecked);

            bfr.SetBit2(0, 2, cbLEDAlarm1.SelectedIndex);
            bfr.SetBit2(0, 6, cbLEDAlarm2.SelectedIndex);
            bfr.SetBit2(1, 2, cbLEDAlarm3.SelectedIndex);
            bfr.SetBit2(1, 6, cbLEDAlarm4.SelectedIndex);
            bfr.SetBit2(2, 2, cbLEDAlarm5.SelectedIndex);
            bfr.SetBit2(2, 6, cbLEDAlarm6.SelectedIndex);
            bfr.SetBit2(3, 2, cbLEDAlarm7.SelectedIndex);
            bfr.SetBit2(3, 6, cbLEDAlarm8.SelectedIndex);
            bfr.SetBit2(4, 2, cbLEDAlarm9.SelectedIndex);
            bfr.SetBit2(4, 6, cbLEDAlarm10.SelectedIndex);
            bfr.SetBit2(5, 2, cbLEDAlarm11.SelectedIndex);

            byte addr = byte.Parse(txtAddr.Text, System.Globalization.NumberStyles.HexNumber);
            SendDataUART((short)(0x480 + addr), 6, bfr);
        }

        private void DispatcherTimerSend_Tick(object sender, EventArgs e)
        {
            //if (isSending) btnSend_Click(null, null);
        }        
    }
}
