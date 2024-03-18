using Microsoft.Win32;
using System.ComponentModel;
using System.Reflection.Metadata;
using System.Runtime.InteropServices;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Animation;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Windows.Threading;

namespace FileManagerGui
{
    public class Util
    {
        public static byte[] SetByte(string str, int len)
        {
            byte[] b = Encoding.Default.GetBytes(str);

            byte[] unicodeBytes = Encoding.Convert(Encoding.Default, Encoding.Unicode, b);
            byte[] s = new byte[len];
            for (int i = 0; i < unicodeBytes.Length; i++)
            {
                s[i] = unicodeBytes[i];
            }
            for (int i = unicodeBytes.Length; i < len; i++)
            {
                s[i] = 0;
            }
            return s;
        }
        public static string GetDirPath(string str)
        {
            var tmp = str.Split('\n');
            str = tmp[1];
            tmp = str.Split('：');//注意这里是中文冒号
            return tmp[1];
        }
    }
    
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {

        private bool blocked = false;
        //声明一个timer用于benchmark
        private System.Timers.Timer timer = new System.Timers.Timer();
        private FileManagerWrapper Wrapper;
        public MainWindow()
        {
            //创建一个文件夹，以当前的时间戳命名
            string timestamp = DateTime.Now.ToString("yyyyMMddHHmmss");
            string baseDir = System.IO.Path.Combine(AppDomain.CurrentDomain.BaseDirectory, timestamp);
            System.IO.Directory.CreateDirectory(baseDir);

            //改变BaseDirectory
            Environment.CurrentDirectory = baseDir;

            InitializeComponent();
            this.Wrapper = new FileManagerWrapper();

        }
        private unsafe void ButtonClicked(object sender, RoutedEventArgs e)
        {
            if (blocked)
            {
                MessageBox.Show("请等待上一个操作完成");
                return;
            }
            var stamp = DateTime.Now;
            Name = (e.OriginalSource as FrameworkElement).Name;
            switch(Name){
                case "btnSelectFile":
                    var openFileDialog = new Microsoft.Win32.OpenFileDialog();
                    openFileDialog.ShowDialog();
                    var fileResult = Util.SetByte(openFileDialog.FileName, 1000 * 4);
                    
                    fixed (Byte* p = &fileResult[0])
                    {
                        var err = Wrapper.OpenCommandFile(p);
                    }
                    this.lbStatus.Content = "正在执行"+Encoding.Unicode.GetString(fileResult);
                    stamp = DateTime.Now;
                    Task.Run(() =>
                    {
                        this.blocked = true;
                        var err = Wrapper.ExecuteCommand();
                        var span = DateTime.Now - stamp;
                        Dispatcher.Invoke(() =>
                        {
                            this.lbStatus.Content = "执行完成,用时"+ span.TotalMilliseconds.ToString() + "ms";
                        });
                        this.blocked = false;
                    });
                    break;
                case "btnScanDir":
                    //将窗口移到一个新的线程

                    var openFolderDialog = new Microsoft.Win32.OpenFolderDialog();
                    openFolderDialog.ShowDialog();
                    var folderResult = Util.SetByte(openFolderDialog.FolderName, 1000 * 4);
                    //设置lbDirSelected的内容为选中的文件夹
                    this.lbDirSelected.Content = openFolderDialog.FolderName;
                    //立即更新lbDirSelected的内容
                    this.lbDirSelected.UpdateLayout();
                    //记录时间戳
                    stamp = DateTime.Now;
                    this.lbStatus.Content = "正在扫描目录...";
                    //更新lbStatus的内容
                    this.lbStatus.UpdateLayout();
                    //打开一个Messagebox，直到扫描结束自动关闭
                    //调用Wrapper的ScanDir方法
                    //Util.ShowLoad(this);
                    Task.Run(() =>
                    {
                        fixed (Byte* p = &folderResult[0])
                        {
                            this.blocked = true;
                            var err = Wrapper.ScanDir(p);
                            var span = DateTime.Now - stamp;
                            Dispatcher.Invoke(() =>
                            {
                                this.lbStatus.Content = "扫描完成，用时：" + span.TotalMilliseconds.ToString() + "ms";
                                //this.listExplorer.Items.Clear();
                            });
                            
                        }
                        
                        Byte* res = Wrapper.GetRoot();
                        byte[] bytes = new byte[1000 * 2];
                        Marshal.Copy((IntPtr)res, bytes, 0, 1000 * 2);
                        string str = Encoding.GetEncoding("Unicode").GetString(bytes);
                        //将\0后面的元素去除
                        str = str.TrimEnd('\ufefe');
                        //this.listExplorer.add
                        Dispatcher.Invoke(() =>
                        {
                            this.listExplorer.Items.Add(str);
                        }
                            );
                        this.blocked = false;
                    }
                    );
                    break;
                case "btnParentDir":
                    //回到上层目录,首先获取上层目录的父节点
                    Byte* res = Wrapper.GetParent();
                    if (res == null)
                    {
                        MessageBox.Show("已经是根目录了");
                        break;
                    }
                    byte[] bytes = new byte[1000 * 2];
                    Marshal.Copy((IntPtr)res, bytes, 0, 1000 * 2);
                    string str = Encoding.GetEncoding("Unicode").GetString(bytes);
                    //将\0后面的元素去除
                    str = str.TrimEnd('\ufefe');
                    //this.listExplorer.add
                    this.listExplorer.Items.Clear();
                    this.listExplorer.Items.Add(str);
                    //然后还要获取父节点的所有兄弟节点
                    res = Wrapper.ProvideSibling();
                    while (res != null)
                    {
                        Marshal.Copy((IntPtr)res, bytes, 0, 1000 * 2);
                        //string str = Encoding.Unicode.GetString(bytes);
                        string strSibling = Encoding.GetEncoding("Unicode").GetString(bytes);
                        //将\0后面的元素去除
                        strSibling = strSibling.TrimEnd('\ufefe');
                        //this.listExplorer.add
                        this.listExplorer.Items.Add(strSibling);
                        res = Wrapper.ProvideSibling();
                    }
                    break;
                case "btnFullDiff":
                    //先检查listView是否有选中的文件夹
                    if (this.listExplorer.SelectedItems.Count == 0)
                    {
                        MessageBox.Show("请先选择一个文件夹!");
                        return;
                    }
                    //获取ListView选中的文件夹的路径
                    string strNow = this.listExplorer.SelectedValue.ToString();
                    this.lbStatus.Content = "正在备份选中文件夹的全量信息";
                    this.lbStatus.UpdateLayout();

                    //设置监视文件夹
                    //根据题目要求，只实现对单个文件夹的监视
                    stamp = DateTime.Now;
                    Task.Run(() =>
                    {
                        strNow = Util.GetDirPath(strNow);
                        byte[] pathResult = Encoding.Unicode.GetBytes(strNow);
                        fixed (Byte* p = &pathResult[0])
                        {
                            this.blocked = true;
                            var err = Wrapper.SetFullDiffNode(p);
                            if (err != 0)
                            {
                                Dispatcher.Invoke(() =>
                                {
                                    this.lbStatus.Content = "备份失败";
                                });
                                this.blocked = false;
                                return;
                            }
                            var span = DateTime.Now - stamp;
                            Dispatcher.Invoke(() =>
                            {
                                this.lbStatus.Content = "备份完成," + "用时"+ span.TotalMilliseconds.ToString() + "ms" ;
                            });
                            this.blocked = false;
                        }
                    });
                    break;
                case "btnBeginFolderStatCmp":
                    this.blocked = true;
                    this.lbStatus.Content = "正在比较文件夹";
                    this.lbStatus.UpdateLayout();
                    stamp = DateTime.Now;
                    Task.Run(() =>
                    {
                        int err = Wrapper.BeginFolderStatCompare();
                        if (err != 0)
                        {
                            Dispatcher.Invoke(() =>
                            {
                                this.lbStatus.Content = "比较失败";
                            });
                            this.blocked = false;
                            return;
                        }
                        var span = DateTime.Now - stamp;
                        Dispatcher.Invoke(() =>
                        {
                            this.lbStatus.Content = "比较完成" + ",用时" + span.TotalMilliseconds.ToString() + "ms";
                        });
                        this.blocked = false;
                    });
                    break;
                case "btnBeginFullDiffCmp":
                    this.blocked = true;
                    this.lbStatus.Content = "正在比较全量信息";
                    this.lbStatus.UpdateLayout();
                    stamp = DateTime.Now;
                    Task.Run(() =>
                    {
                        var err = Wrapper.BeginFullDiffCompare();
                        if (err != 0)
                        {
                            Dispatcher.Invoke(() =>
                            {
                                this.lbStatus.Content = "比较失败";
                            });
                            this.blocked = false;
                            return;
                        }
                        var span = DateTime.Now - stamp;
                        Dispatcher.Invoke(() =>
                        {
                            this.lbStatus.Content = "比较完成，" + "用时" + span.TotalMilliseconds.ToString() + "ms";
                        });
                        this.blocked = false;
                    });
                    break;
                default:
                    break;
            }
            
        }
        

        private unsafe void ListView_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            if (this.listExplorer.SelectedItems.Count != 0)
            {
                this.textblockDir.Text = Util.GetDirPath(this.listExplorer.SelectedValue.ToString());
            }
            /*
            Byte* res = Wrapper.GetRoot();
            byte[] bytes = new byte[1000*2];    
            Marshal.Copy((IntPtr)res, bytes, 0, 1000*2);
            //string str = Encoding.Unicode.GetString(bytes);
            string str = Encoding.GetEncoding("Unicode").GetString(bytes);
            //将\0后面的元素去除
            str = str.TrimEnd('\ufefe');
            //this.listExplorer.add
            this.listExplorer.Items.Add(str);*/
        }
        private unsafe void Listview_MouseDoubleClicked(object sender, MouseButtonEventArgs e)
        {
            //双击，首先获取孩子节点
            //获取选中的item里的文本
            string strNow = this.listExplorer.SelectedValue.ToString();
            //用正则匹配找到"路径-(Path) "的内容
            strNow = Util.GetDirPath(strNow);
            Byte* res;
            var pathResult = Encoding.Unicode.GetBytes(strNow);
            //var pathResult = Util.SetByte(str, 1000 * 4);
            fixed (Byte* p = &pathResult[0])
            {
                res = Wrapper.GetChild(p);
                if(res == null)
                {
                    MessageBox.Show("这个文件夹没有子目录");
                    return;
                }
            }
            //清除listExplorer的所有item
            this.listExplorer.Items.Clear();
            //添加新的Item
            byte[] bytes = new byte[1000 * 2];
            Marshal.Copy((IntPtr)res, bytes, 0, 1000 * 2);
            //string str = Encoding.Unicode.GetString(bytes);
            string strChild = Encoding.GetEncoding("Unicode").GetString(bytes);
            //将\0后面的元素去除
            strChild = strChild.TrimEnd('\ufefe');
            //this.listExplorer.add
            this.listExplorer.Items.Add(strChild);

            //然后还要获取孩子节点的所有兄弟节点
            res = Wrapper.ProvideSibling();
            while (res != null)
            {
                Marshal.Copy((IntPtr)res, bytes, 0, 1000 * 2);
                //string str = Encoding.Unicode.GetString(bytes);
                string strSibling = Encoding.GetEncoding("Unicode").GetString(bytes);
                //将\0后面的元素去除
                strSibling = strSibling.TrimEnd('\ufefe');
                //this.listExplorer.add
                this.listExplorer.Items.Add(strSibling);
                res = Wrapper.ProvideSibling();
            }
            /*
            Byte* res = Wrapper.GetRoot();
            byte[] bytes = new byte[1000*2];    
            Marshal.Copy((IntPtr)res, bytes, 0, 1000*2);
            //string str = Encoding.Unicode.GetString(bytes);
            string str = Encoding.GetEncoding("Unicode").GetString(bytes);
            //将\0后面的元素去除
            str = str.TrimEnd('\ufefe');
            //this.listExplorer.add
            this.listExplorer.Items.Add(str);*/
        }
    }

}