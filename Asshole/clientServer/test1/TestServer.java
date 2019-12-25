package test1;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.net.InetSocketAddress;
import java.net.ServerSocket;
import java.net.Socket;

/**
 * ソケット通信(サーバー側)
 */
class Sample1 {

	void runSample() {

	ServerSocket sSocket = null;
	Socket socket = null;
	BufferedReader reader = null; //clientからserverに送られてきた文字列
	PrintWriter writer = null; //serverからclientへ送信するもじれつ

	try{
		//IPアドレスとポート番号を指定してサーバー側のソケットを作成
		sSocket = new ServerSocket();
		sSocket.bind(new InetSocketAddress
				("127.0.0.1",8765));

		//System.out.println("クライアントからの入力待ち状態");

		//クライアントからの要求を待ち続けます
		socket = sSocket.accept();

		//クライアントからの受取用
		reader = new BufferedReader(
				new InputStreamReader
				(socket.getInputStream()));

		//サーバーからクライアントへの送信用
		writer = new PrintWriter(
				socket.getOutputStream(), true);

		//無限ループ　byeの入力でループを抜ける
		String line = null;
		int num;
        while (true) {

        	line = reader.readLine();

        	if (line.equals("bye")) {
                break;
        	}

					//数字ならここｔｒｙ
        	try{
        		num = Integer.parseInt(line); //クライアントの送信

	        	if(num%2==0){
	            	//送信用の文字を送信
	                writer.println("OK");
	        	}else{
	            	//送信用の文字を送信
	                writer.println("NG");
	        	}
        	}catch(NumberFormatException e){
        		//送信用の文字を送信
                writer.println("数値を入力して下さい");
        	}

            //System.out.println("クライアントで入力された文字＝" + line);//clientからの入力を表示
        }
			}catch(Exception e){
				e.printStackTrace();
			}finally{ //byeが入力されたとき、
				try{
					if (reader!=null){
							reader.close();
						}
						if (writer!=null){
							writer.close();
						}
						if (socket!=null){
							socket.close();
						}
						if (sSocket!=null){
							sSocket.close();
						}
						//System.out.println("サーバー側終了です");
				} catch (IOException e) {
					e.printStackTrace();
			}
		}
	}
}

public class TestServer {
	public static void main(String[] args) {
		Sample1 s1 = new Sample1(); //　①　Sample1をnew
		s1.runSample(); // ② Sample1のrunSampleを呼ぶ
	}
}
