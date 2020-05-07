using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO.Ports;
using System.Threading;

/*namespace ConsoleApp1
{
    class Program
    {
        static void Main(string[] args)
        {
        }
    }
}*/

public class PortChat
{
    
    static bool continue_or;//创建一个用于判定程序是否继续的bool类实体
    static SerialPort serialPort1;//创建了一个新的SerialPort类实体

    public static void Main()
    {
        string name;
        string message;
        StringComparer stringComparer = StringComparer.OrdinalIgnoreCase;
        Thread readThread = new Thread(Read);

        // 初始化 SerialPort 类的新实例。
        serialPort1 = new SerialPort();

        // 允许用户设置适当的属性。
        serialPort1.PortName = SetPortName(serialPort1.PortName);
        serialPort1.BaudRate = SetPortBaudRate(serialPort1.BaudRate);
        serialPort1.Parity = SetPortParity(serialPort1.Parity);
        serialPort1.DataBits = SetPortDataBits(serialPort1.DataBits);
        serialPort1.StopBits = SetPortStopBits(serialPort1.StopBits);
        serialPort1.Handshake = SetPortHandshake(serialPort1.Handshake);

        // 设置读写超时
        serialPort1.ReadTimeout = 500;//获取或设置读取操作未完成时发生超时之前的毫秒数
        serialPort1.WriteTimeout = 500;//获取或设置写入操作未完成时发生超时之前的毫秒数

        serialPort1.Open();//打开一个新的串行端口连接
        continue_or = true;
        readThread.Start();//导致操作系统将当前实例的状态改为Running

        Console.Write("端口名: ");//将指定的字符串写入标准输出流
        name = Console.ReadLine();//从标准输入流读取下一行字符

        Console.WriteLine("输入quit来退出：");

        while (continue_or)
        {
            message = Console.ReadLine();

            if (stringComparer.Equals("quit", message))//读取到quit就准备退出
            {
                continue_or = false;
            }
            else
            {
                string time;
                Console.WriteLine("Send <{0}>: {1},{2}", name, message, time = DateTime.Now.TimeOfDay.ToString());
              
                serialPort1.WriteLine(//将指定的字符串和NewLine值写入输入缓存区
                    String.Format("Receive <{0}>: {1},{2}", name, message, time = DateTime.Now.TimeOfDay.ToString()));
                //将指定字符串中的格式项替换为两个指定对象的字符串表示形式 
            }
        }

        readThread.Join();//阻止调用线程直到线程终止，同时继续执行标准的COM和SendMesage发送
        serialPort1.Close();//关闭端口连接
    }

    public static void Read()
    {
        while (continue_or)
        {
            try
            {
                string message = serialPort1.ReadLine();//一直读取到输入缓冲区的SerialPort.NewLine值
                Console.WriteLine(message);
            }
            catch (TimeoutException) { }//当为进程或操作分配的时间已过期时引发的异常
        }
    }

    // 显示端口值并提示用户输入端口。
    public static string SetPortName(string defaultPortName)
    {
        string portName;

        Console.WriteLine("可用端口:");
        foreach (string s in SerialPort.GetPortNames())//GetPortNames：获取当前计算机的串行端口名的数组
        {
            Console.WriteLine("   {0}", s);//使用指定的格式信息，将指定对象（后跟当前行中止符）的文本表示形式写入标准输出流
        }

        Console.Write("输入端口名 (默认: {0}): ", defaultPortName);
        portName = Console.ReadLine();

        if (portName == "" || !(portName.ToLower()).StartsWith("com"))//不输入或者开头不是com就表示默认值了
        {
            portName = defaultPortName;
        }
        return portName;
    }
    // 显示波特率值并提示用户输入一个值。
    public static int SetPortBaudRate(int defaultPortBaudRate)
    {
        string baudRate;

        Console.Write("波特率(默认:{0}): ", defaultPortBaudRate);
        baudRate = Console.ReadLine();

        if (baudRate == "")
        {
            baudRate = defaultPortBaudRate.ToString();
        }

        return int.Parse(baudRate);
    }

    // 显示端口parity值并提示用户输入一个值。
    public static Parity SetPortParity(Parity defaultPortParity)
    {
        string parity;

        Console.WriteLine("可用的 奇偶检验 选择:");
        foreach (string s in Enum.GetNames(typeof(Parity)))
        {
            Console.WriteLine("   {0}", s);
        }

        Console.Write("输入 奇偶检验 值 (默认: {0}):", defaultPortParity.ToString(), true);
        parity = Console.ReadLine();

        if (parity == "")
        {
            parity = defaultPortParity.ToString();
        }

        return (Parity)Enum.Parse(typeof(Parity), parity, true);
    }
    // 显示数据位的值并提示用户输入一个值。
    public static int SetPortDataBits(int defaultPortDataBits)
    {
        string dataBits;

        Console.Write("输入数据位的值 (默认: {0}): ", defaultPortDataBits);
        dataBits = Console.ReadLine();

        if (dataBits == "")
        {
            dataBits = defaultPortDataBits.ToString();
        }

        return int.Parse(dataBits.ToUpperInvariant());
    }

    // 显示停止位的值并提示用户输入一个值。
    public static StopBits SetPortStopBits(StopBits defaultPortStopBits)
    {
        string stopBits;

        Console.WriteLine("可用的结束位选择:");
        foreach (string s in Enum.GetNames(typeof(StopBits)))
        {
            Console.WriteLine("   {0}", s);
        }

        Console.Write("输入结束位的值 (None is not supported and \n" +
         "raises an ArgumentOutOfRangeException. \n (Default: {0}):", defaultPortStopBits.ToString());
        stopBits = Console.ReadLine();

        if (stopBits == "")
        {
            stopBits = defaultPortStopBits.ToString();
        }

        return (StopBits)Enum.Parse(typeof(StopBits), stopBits, true);
    }
    // 显示可用的交互选择并提示用户输入一个值。
    public static Handshake SetPortHandshake(Handshake defaultPortHandshake)
    {
        string handshake;

        Console.WriteLine("可用的交互选择:");
        foreach (string s in Enum.GetNames(typeof(Handshake)))
        {
            Console.WriteLine("   {0}", s);
        }

        Console.Write("输入交互选择 (默认: {0}):", defaultPortHandshake.ToString());
        handshake = Console.ReadLine();

        if (handshake == "")
        {
            handshake = defaultPortHandshake.ToString();
        }

        return (Handshake)Enum.Parse(typeof(Handshake), handshake, true);
    }
}
