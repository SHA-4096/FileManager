using System.Reflection.Metadata;
using System.Text;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

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
    }
    
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        private FileManagerWrapper Wrapper;
        public MainWindow()
        {
            InitializeComponent();
            this.Wrapper = new FileManagerWrapper();
        }
        private unsafe void ButtonClicked(object sender, RoutedEventArgs e)
        {
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
                    Wrapper.ExecuteCommand();
                    break;
                case "btnScanDir":
                    var openFolderDialog = new Microsoft.Win32.OpenFolderDialog();
                    openFolderDialog.ShowDialog();
                    var folderResult = Util.SetByte(openFolderDialog.FolderName, 1000 * 4);
                    fixed (Byte* p = &folderResult[0])
                    {
                        var err = Wrapper.ScanDir(p);
                    }
                    break;
            }
            
        }
    }

}