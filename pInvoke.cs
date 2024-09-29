using System;
using System.Runtime.InteropServices;

public class Messages
{
	static void Main()
	{
		 /* THIS CODE DOESN'T WORK
		  [DllImport("user32.dll", CharSet = CharSet.Auto)]
		 static extern IntPtr SendMessage(IntPtr hWnd, uint Msg,
																nuint wParam,  nint lParam);

		SendMessage(new IntPtr(0xffff), 0x004A, 0 , 0); */
	}
}