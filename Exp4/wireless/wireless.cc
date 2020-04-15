/*
	Networks Assignment 4 Application #2 Wireless Connection 
	
	All traces can be found in the 'wireless' folder
	All plots of the simulation can be found in the 'plots' folder.

	THE NETWORK TOPOLOGY IS AS FOLLOWS : 	
 *
 *    STA       Ap        Ap       STA
 *     *        *         *        *
 *     |        |         |        |
 *   node0     BSS1------BSS2     node1
 *         100      10Mbps    100
 *         Mbps     100 ms    Mbps
 *         20ms               20ms
 *        (50ns)             (50ns)
 
   	Note: the delay value for wireless links. 
   	20 ms value implies a distance of 20e-3 * 3e8 = 60e5 which is outside wireless range. Thus we made suitable adjustments.
 	Hence we assumed a spacing of 50m that would mean a propogation delay of approx 166 ns.
 
*/


#include <string>
#include<vector>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/drop-tail-queue.h"
#include "ns3/mobility-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/wifi-module.h"

// The ns3 project has been created in a namespace called 'ns3' 
using namespace ns3;

// We define the Component, this will be later used to diplay info messages on the terminal
NS_LOG_COMPONENT_DEFINE ("WirelessConnection") ;

int main( int argc , char ** argv )
{
	// In order to display proper messages on the terminal, we enable the Component we defined above
	LogComponentEnable("WirelessConnection" , LOG_INFO);

	std::string tcpAgent = "TcpWestwood";
	std::vector<int> packetSizes = {40, 44, 48, 52, 60, 250, 300, 552, 576, 628, 1420 ,1500};

	// Commandline arguments are used to take the TCP Agent from the user
	CommandLine cmd;
	cmd.AddValue("TcpAgent" , "Specifies the TCP Agent to be used, available options are TCP Westwood,TCP Veno and TCP Vegas" , tcpAgent);
	cmd.Parse(argc,argv);

	// Based on the input from the user, we set the TCP Agent appropriately. As given in the question 
	// the user has an option to choose among TCP Veno, TCP Vegas and TCP Westwood

	if( tcpAgent == "Westwood" )
    	Config::SetDefault("ns3::TcpL4Protocol::SocketType", TypeIdValue (TcpWestwood::GetTypeId()));
	else if( tcpAgent == "Veno")
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVeno"));
	else if( tcpAgent == "Vegas")
		Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVegas"));
	else{
		NS_LOG_INFO("Kindly enter a correct TCP Agent!!");
		exit(0);
	}

	NS_LOG_INFO("Using TCP AGENT : ");
	NS_LOG_INFO(tcpAgent);

	// We will be generating our traces in ASCII Format, for that we use ASCII Helper
	// We could also have used PCAP helper to generate traces in PCAP Format which can be read from the command tcpdump
  	AsciiTraceHelper asciiTraceHelper;
	std::string traceFileName = "traceWirelessTcp" + tcpAgent + ".txt";

	// This is the output stream, all the relevant data will be pur in this stream
  	Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (traceFileName);
    *stream->GetStream () << "Using TCP Agent: Tcp" << tcpAgent << "\n";
    *stream->GetStream () <<"Packet Size \t\tThroughput \t\tFairness Index " << "\n";
	
	// setting drop tail policy
	Config::SetDefault ("ns3::WifiMacQueue::DropPolicy", EnumValue(WifiMacQueue::DROP_NEWEST));

    // Iterating through all the given packet sizes
	for( int index = 0 ; index<packetSizes.size() ; index++)
	{
		int packetSize = packetSizes[index];
        
        // We set the segment size equal to the packet size 
        Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (packetSize));

		NS_LOG_INFO("Creating nodes");

		// NodeContainer is a topolohy helper which stores all the nodes. As per question,
		// we create 4 nodes, 2 of which will be routers
		NodeContainer nodes;
		nodes.Create(4);

		// We create NodeContainers to contain Ap Nodes and Sta Nodes
		NodeContainer wifiApNode_1 = nodes.Get (1);
		NodeContainer wifiApNode_2 = nodes.Get (2);
		NodeContainer wifiStaNodes_1 = nodes.Get(0);
		NodeContainer wifiStaNodes_2 = nodes.Get(3);

		// There are 2 main layers in wireless connection namely, PHY layer and MAC layer.
		// We first create the PHY Layers and create the underlying channel.
		// All stanodes of a given wireless connection will be connected to the same channel.
		YansWifiChannelHelper channel_1 = YansWifiChannelHelper::Default ();
		YansWifiChannelHelper channel_2 = YansWifiChannelHelper::Default ();
  		YansWifiPhyHelper phy_1 = YansWifiPhyHelper::Default ();
  		YansWifiPhyHelper phy_2 = YansWifiPhyHelper::Default ();
  		phy_1.SetChannel (channel_1.Create ());
  		phy_2.SetChannel (channel_2.Create ());

  		//Once the PHY helper is configured, we then focus on the MAC layer. 
  		// The SetRemoteStationManager method tells the helper the type of rate control algorithm to use. Here, it is
		// asking the helper to use the AARF algorithm 
		WifiHelper wifi;
  		wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  		// Then, we configure the type of MAC, the SSID of the infrastructure network we want to setup
  		WifiMacHelper mac;
  		Ssid ssid = Ssid ("ns-3-ssid");
  		mac.SetType ( "ns3::StaWifiMac" , "Ssid" , SsidValue (ssid), "ActiveProbing" , BooleanValue (false) ) ;

  		// We create netdevice containers for both staNodes
  		NetDeviceContainer staDevices_1;
  		NetDeviceContainer staDevices_2;

  		// and then install the mac layer and phy layer in the stanodes
  		staDevices_1 = wifi.Install( phy_1 , mac , wifiStaNodes_1 );
  		staDevices_2 = wifi.Install( phy_2 , mac , wifiStaNodes_2 );

  		// We now specify that we will be now configuring the Access Point nodes. 
  		mac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid) );

  		// and just like in the case of sta nodes, here also we create the netdevice containers and 
  		// install the set the phy and mac layer. 
  		NetDeviceContainer apDevices_1;
  		NetDeviceContainer apDevices_2;

  		apDevices_1 = wifi.Install( phy_1 , mac , wifiApNode_1);
  		apDevices_2 = wifi.Install( phy_2 , mac , wifiApNode_2);

  		// We would now use the mobility helper 
  		// We are assuming that all our nodes are fixed, ie they are not moving
  		// So we provide their fixed positions in a vector, and as specified above, we keep the distance of 
  		// our device from the Ap Node to be 50m. For the sake of generality we have kept all
  		// nodes at equal distances.
	    MobilityHelper mobility;
	    Ptr<ListPositionAllocator> locationVector = CreateObject<ListPositionAllocator> ();
	    locationVector->Add (Vector (000.0, 0.0, 0.0));
	    locationVector->Add (Vector (050.0, 0.0, 0.0));
	    locationVector->Add (Vector (100.0, 0.0, 0.0));
	    locationVector->Add (Vector (150.0, 0.0, 0.0));

	    mobility.SetPositionAllocator (locationVector);
	    mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
	    mobility.Install (nodes.Get(0));
	    mobility.Install (nodes.Get(1));
	    mobility.Install (nodes.Get(2));
	    mobility.Install (nodes.Get(3));

	    // We now connect the 2 base stations using a point to point connection 
	    // also providing the required values of datarate and delay.
	    // We also set the drop tail queue
	    PointToPointHelper bsaToBsa;
		bsaToBsa.SetDeviceAttribute("DataRate" , StringValue("10Mbps") );
		bsaToBsa.SetChannelAttribute("Delay" , StringValue("100ms") );
        bsaToBsa.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize("125000B")));  

	    NetDeviceContainer baseStations;
	    baseStations = bsaToBsa.Install(nodes.Get(1) , nodes.Get(2) );

	    // We then initialize the internet stack in all the nodes.
	    InternetStackHelper internet;
	    internet.Install (nodes);

	  	// Next, we assign IP Addresses to all kinds of network device drivers.
	    // We set the base and subnet and pass the NetDeviceContainer as an argument.
	    NS_LOG_INFO( "Assigning IP Addresses." );
  		Ipv4AddressHelper ipv4;
  		ipv4.SetBase( "10.1.1.0" , "255.255.255.0" );
	  	Ipv4InterfaceContainer interfaces_1 = ipv4.Assign ( apDevices_1 );
	  	Ipv4InterfaceContainer interfaces_3 = ipv4.Assign ( staDevices_1 );
	  	
  		ipv4.SetBase( "10.1.2.0" , "255.255.255.0" );
	  	Ipv4InterfaceContainer interfaces_2 = ipv4.Assign ( apDevices_2 );
	  	Ipv4InterfaceContainer interfaces_4 = ipv4.Assign ( staDevices_2 );

  		ipv4.SetBase( "10.1.3.0" , "255.255.255.0" );
	  	Ipv4InterfaceContainer interfaces_5 = ipv4.Assign ( baseStations );


	  	// We initialize the ApplicationContainer which will contain the application
	  	// that will be later installed in the source and sink of the topology.
	  	ApplicationContainer sourceApps;
	  	ApplicationContainer sinkApps;
		
		//Since we have actually built an internetwork here, we need some form of internetwork routing. 
		// So we use global routing. Global routing takes advantage of the fact that the entire internetwork is accessible
		//in the simulation and runs through the all of the nodes created for the simulation — it does the hard work of setting up
		//routing 
		// To use globalrouting we write the next line. 
		Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	    NS_LOG_INFO( "Assigned IP Addresses." );

	    uint16_t port = 10000 + index;

      	// Now from the source side, we had 3 options :
	    // to make a custom application as done in fourth.cc of examples/tutorial
	    // to make a bulk application or to make a onoff application
	    // Since we didnt had to deal with callbacks in the assignment, we have used bulksendapplication
	    // which tries to send as much traffic as possible thus utilizing maximum bandwidth.
      	BulkSendHelper source ("ns3::TcpSocketFactory", InetSocketAddress (interfaces_4.GetAddress (0), port));
      	source.SetAttribute("SendSize" , UintegerValue(packetSize));
      	source.SetAttribute("MaxBytes" , UintegerValue(0));
      	sourceApps= (source.Install( nodes.Get(0) ) );

		// Since we are using TCP, we need something on the destination node to receive TCP connections and data.
		// For this purpose we use a PacketSink Application 
		PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
		sinkApps= (sink.Install(nodes.Get(3)));

		// We assign the start time and stop time of all applications.
		sinkApps.Start(Seconds(0));
		sinkApps.Stop(Seconds(5));
		sourceApps.Start (Seconds (0));
	  	sourceApps.Stop (Seconds (5));

 		// FlowMonitor is used to monitor our network and can be used to provide statistics of the network
 		FlowMonitorHelper flowmon;
		Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

		// We start our simulation after providing the max time the simulation is allowed to run.
 		Simulator::Stop (Seconds (5));
  		Simulator::Run ();

  		// The following section of code is used to produce all statistics of the network
		Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  		FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  		std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin();

  		// this is the time elapsed between the events of first time when the TX packet is transferred
  		// and the time when the last RX packet is received 
	  	double time = i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds();
		double throughput = (((i->second.rxBytes*8)/time)/1000);

      	// We use the following variables to calculate the Jain's fairness index.
      	double sumThrougput = 0 , sumSquareThroughput = 0;

      	sumThrougput += throughput;
      	sumSquareThroughput += throughput*throughput;
      	double fairnessIndex = (sumThrougput * sumThrougput )/ (sumSquareThroughput);
		
      	// We output the values in the file we created before.
		*stream->GetStream () <<  std::to_string(packetSize) << "\t\t\t" << std::to_string(throughput) <<"\t\t"<<fairnessIndex << "\n";
      	std::cout <<  std::to_string(packetSize) << "\t\t\t" << (((i->second.rxBytes*8)/time)/1000)<<"\t\t"<<fairnessIndex << "\n";
  		
      	// After the simulation stops, we free all objects created by using the function Destroy.
  		Simulator::Destroy ();
	}
	return 0;

}