//
// MainPage.xaml.h
// Declaration of the MainPage class.
//

#pragma once

#include "MainPage.g.h"

namespace kdeconnect_uwp
{
	/// <summary>
	/// An empty page that can be used on its own or navigated to within a Frame.
	/// </summary>
	public ref class MainPage sealed
	{
	public:
		MainPage();

	private:
		Windows::Networking::Sockets::DatagramSocket^ listenerSocket;

		void StartListenerSocket();
		void CloseListenerSocket();
		void MessageReceived(
			Windows::Networking::Sockets::DatagramSocket^ socket,
			Windows::Networking::Sockets::DatagramSocketMessageReceivedEventArgs^ eventArguments);

		void buttonSetHostName_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void buttonRefresh_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void buttonUnPair_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void buttonSendPing_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e);
		void textBoxHostName_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e);
	};
}
