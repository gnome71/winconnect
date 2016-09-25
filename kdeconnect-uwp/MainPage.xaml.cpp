//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

#define REMOTE_ADDR = "255.255.255.255"
#define LOCAL_PORT = "1716"

using namespace kdeconnect_uwp;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage() :
	listenerSocket(nullptr)
{
	InitializeComponent();

	auto view = Windows::UI::ViewManagement::ApplicationView::GetForCurrentView();
	view->Title = "KDE Connect";
	kdeconnect_uwp::MainPage::textBlockDebugOutput->Text = "In MainPage() Constructor.\n";

	StartListenerSocket();
}


void kdeconnect_uwp::MainPage::StartListenerSocket()
{
	kdeconnect_uwp::MainPage::textBlockDebugOutput->Text += "StartListenerSocket() called on port: 1716\n";
	listenerSocket = ref new DatagramSocket();

	// TODO: Start listen operation: see DatagramSocket example line 152
	//create_task
}


void kdeconnect_uwp::MainPage::CloseListenerSocket()
{
	if (listenerSocket != nullptr)
	{
		delete listenerSocket;
		listenerSocket = nullptr;
	}
}


void kdeconnect_uwp::MainPage::buttonSetHostName_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}


void kdeconnect_uwp::MainPage::buttonRefresh_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	kdeconnect_uwp::MainPage::textBlockDebugOutput->Text += "buttonRefresh clicked.\n";
	// FIXME:
	//HostName^ serverHost = Windows::Networking::Connectivity::NetworkInformation::GetHostNames();
	HostName^ serverHost = ref new HostName("127.0.0.1");
	//kdeconnect_uwp::MainPage::textBoxHostName->Text = serverHost->Fi;
}


void kdeconnect_uwp::MainPage::buttonUnPair_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}


void kdeconnect_uwp::MainPage::buttonSendPing_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{

}


void kdeconnect_uwp::MainPage::textBoxHostName_TextChanged(Platform::Object^ sender, Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{

}
