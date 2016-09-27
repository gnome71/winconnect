//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"

using namespace kdeconnect_uwp;

using namespace Platform;
using namespace concurrency;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Networking;
using namespace Windows::Networking::Sockets;
using namespace Windows::System::Threading;
using namespace Windows::UI::Core;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

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
	listenerSocket->MessageReceived += ref new TypedEventHandler<DatagramSocket^, DatagramSocketMessageReceivedEventArgs^>(
		this,
		&kdeconnect_uwp::MainPage::MessageReceived);

	// TODO: Start listen operation: see DatagramSocket example line 152
	create_task(listenerSocket->BindServiceNameAsync("1716")).then(
		[=](task<void> bindTask)
	{
		try
		{
			// Try getting an exception
			bindTask.get();

			kdeconnect_uwp::MainPage::textBlockDebugOutput->Text += "Listener Task running.\n";
			kdeconnect_uwp::MainPage::textBlockDebugOutput->Text += "Listening localIP:    " + listenerSocket->Information->LocalAddress + "\n";
			kdeconnect_uwp::MainPage::textBlockDebugOutput->Text += "Listening localPort:  " + listenerSocket->Information->LocalPort + "\n";
			kdeconnect_uwp::MainPage::textBlockDebugOutput->Text += "Listening remoteIp:   " + listenerSocket->Information->RemoteAddress + "\n";
			kdeconnect_uwp::MainPage::textBlockDebugOutput->Text += "Listening remotePort: " + listenerSocket->Information->RemotePort + "\n";
		}
		catch (Exception^ exception)
		{
			delete listenerSocket;
			listenerSocket = nullptr;

			kdeconnect_uwp::MainPage::textBlockDebugOutput->Text += "Creating Listener Task failed with:  " + exception->Message + "\n";
		}
	});
}


void kdeconnect_uwp::MainPage::CloseListenerSocket()
{
	if (listenerSocket != nullptr)
	{
		delete listenerSocket;
		listenerSocket = nullptr;
	}
}


void kdeconnect_uwp::MainPage::MessageReceived(
	Windows::Networking::Sockets::DatagramSocket ^ socket, 
	Windows::Networking::Sockets::DatagramSocketMessageReceivedEventArgs ^ eventArguments)
{
	unsigned int msgLength = eventArguments->GetDataReader()->UnconsumedBufferLength;
	String^ rcvMsg = eventArguments->GetDataReader()->ReadString(msgLength);
	Dispatcher->RunAsync(CoreDispatcherPriority::Normal, ref new DispatchedHandler([this, rcvMsg]()
	{
		kdeconnect_uwp::MainPage::textBlockDebugOutput->Text += "Message Received.\n";
		kdeconnect_uwp::MainPage::textBlockDebugOutput->Text += rcvMsg;
	}));
}


void kdeconnect_uwp::MainPage::buttonSetHostName_Click(
	Platform::Object^ sender, 
	Windows::UI::Xaml::RoutedEventArgs^ e)
{
		throw ref new Platform::NotImplementedException();
}


void kdeconnect_uwp::MainPage::buttonRefresh_Click(
	Platform::Object^ sender, 
	Windows::UI::Xaml::RoutedEventArgs^ e)
{
	kdeconnect_uwp::MainPage::textBlockDebugOutput->Text += "buttonRefresh clicked.\n";
	// TODO: Get computer name to use for pairing
	//HostName^ serverHost = Windows::Networking::Connectivity::NetworkInformation::GetHostNames();
	HostName^ serverHost = ref new HostName("127.0.0.1");
	//kdeconnect_uwp::MainPage::textBoxHostName->Text = serverHost->Fi;
}


void kdeconnect_uwp::MainPage::buttonUnPair_Click(
	Platform::Object^ sender, 
	Windows::UI::Xaml::RoutedEventArgs^ e)
{
	throw ref new Platform::NotImplementedException();
}


void kdeconnect_uwp::MainPage::buttonSendPing_Click(
	Platform::Object^ sender, 
	Windows::UI::Xaml::RoutedEventArgs^ e)
{
	throw ref new Platform::NotImplementedException();
}


void kdeconnect_uwp::MainPage::textBoxHostName_TextChanged(
	Platform::Object^ sender, 
	Windows::UI::Xaml::Controls::TextChangedEventArgs^ e)
{
	throw ref new Platform::NotImplementedException();
}
