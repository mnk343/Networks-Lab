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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WirelessConnection") ;

int main( int argc , char ** argv )
{
	LogComponentEnable("WirelessConnection" , LOG_INFO);

	std::string tcpAgent = "TcpWestwood";
	std::vector<int> packetSizes = {40, 44, 48, 52, 60, 250, 300, 552, 576, 628, 1420 ,1500};

	CommandLine cmd;
	cmd.AddValue("TcpAgent" , "Specifies the TCP Agent to be used, available options are TCP Westwood,TCP Veno and TCP Vegas" , tcpAgent);
	cmd.Parse(argc,argv);

		// Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpWestwood"));

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

  	AsciiTraceHelper asciiTraceHelper;
	std::string traceFileName = "traceWirelessTcp" + tcpAgent + ".txt";

  	Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (traceFileName);
    *stream->GetStream () << "Using TCP Agent: Tcp" << tcpAgent << "\n";
    *stream->GetStream () <<"Packet Size \t\tThroughput \t\tFairness Index " << "\n";
	Config::SetDefault ("ns3::WifiMacQueue::DropPolicy", EnumValue(WifiMacQueue::DROP_NEWEST));
 
	for( int index = 0 ; index<packetSizes.size() ; index++)
	{
		int packetSize = packetSizes[index];
        
        Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (packetSize));

		NS_LOG_INFO("Creating nodes");
		NodeContainer nodes;
		nodes.Create(4);

		NodeContainer wifiApNode_1 = nodes.Get (1);
		NodeContainer wifiApNode_2 = nodes.Get (2);
		NodeContainer wifiStaNodes_1 = nodes.Get(0);
		NodeContainer wifiStaNodes_2 = nodes.Get(3);

		YansWifiChannelHelper channel_1 = YansWifiChannelHelper::Default ();
		YansWifiChannelHelper channel_2 = YansWifiChannelHelper::Default ();
  		YansWifiPhyHelper phy_1 = YansWifiPhyHelper::Default ();
  		YansWifiPhyHelper phy_2 = YansWifiPhyHelper::Default ();
  		phy_1.SetChannel (channel_1.Create ());
  		phy_2.SetChannel (channel_2.Create ());

		WifiHelper wifi;
  		wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  		WifiMacHelper mac;
  		Ssid ssid = Ssid ("ns-3-ssid");

  		mac.SetType ( "ns3::StaWifiMac" , "Ssid" , SsidValue (ssid), "ActiveProbing" , BooleanValue (false) ) ;

  		NetDeviceContainer staDevices_1;
  		NetDeviceContainer staDevices_2;

  		staDevices_1 = wifi.Install( phy_1 , mac , wifiStaNodes_1 );
  		staDevices_2 = wifi.Install( phy_2 , mac , wifiStaNodes_2 );

  		mac.SetType ("ns3::ApWifiMac","Ssid", SsidValue (ssid) );

  		NetDeviceContainer apDevices_1;
  		NetDeviceContainer apDevices_2;

  		apDevices_1 = wifi.Install( phy_1 , mac , wifiApNode_1);
  		apDevices_2 = wifi.Install( phy_2 , mac , wifiApNode_2);

  		// MobilityHelper mobility;

  		// mobility.SetPositionAllocator ( "ns3::GridPositionAllocator",
    //                              	    "MinX", DoubleValue (0.0),
    //                              		"MinY", DoubleValue (0.0),
    //                              		"DeltaX", DoubleValue (5.0),
    //                              		"DeltaY", DoubleValue (10.0),
    //                              		"GridWidth", UintegerValue (3),
    //                              		"LayoutType", StringValue ("RowFirst"));

	  	// mobility.SetMobilityModel    (  "ns3::RandomWalk2dMobilityModel",
	   //                           		"Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
	  	// mobility.Install (wifiStaNodes_1);
	  	// mobility.Install (wifiStaNodes_2);


  		// mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  		// mobility.Install (wifiApNode_1);
  		// mobility.Install (wifiApNode_2);

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

	    PointToPointHelper bsaToBsa;
		bsaToBsa.SetDeviceAttribute("DataRate" , StringValue("10Mbps") );
		bsaToBsa.SetChannelAttribute("Delay" , StringValue("100ms") );
        bsaToBsa.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize("125000B")));  

	    NetDeviceContainer baseStations;
	    baseStations = bsaToBsa.Install(nodes.Get(1) , nodes.Get(2) );

	    InternetStackHelper internet;
	    internet.Install (nodes);

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

	  	ApplicationContainer sourceApps;
	  	ApplicationContainer sinkApps;
	    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
	    NS_LOG_INFO( "Assigned IP Addresses." );

	    uint16_t port = 10000 + index;

      	BulkSendHelper source ("ns3::TcpSocketFactory", InetSocketAddress (interfaces_4.GetAddress (0), port));
      	source.SetAttribute("SendSize" , UintegerValue(packetSize));
      	source.SetAttribute("MaxBytes" , UintegerValue(0));
      	sourceApps= (source.Install( nodes.Get(0) ) );

		PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
		sinkApps= (sink.Install(nodes.Get(3)));

		sinkApps.Start(Seconds(0));
		sinkApps.Stop(Seconds(10));
		sourceApps.Start (Seconds (0));
	  	sourceApps.Stop (Seconds (10));
 		
 		FlowMonitorHelper flowmon;
		Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

 		Simulator::Stop (Seconds (10));
  		Simulator::Run ();

		Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  		FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  		std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin();

	  	double time = i->second.timeLastRxPacket.GetSeconds() - i->second.timeFirstTxPacket.GetSeconds();
		double throughput = (((i->second.rxBytes*8)/time)/1000);
      	
      	double sumThrougput = 0 , sumSquareThroughput = 0;

      	sumThrougput += throughput;
      	sumSquareThroughput += throughput*throughput;
      	double fairnessIndex = (sumThrougput * sumThrougput )/ (sumSquareThroughput);
		*stream->GetStream () <<  std::to_string(packetSize) << "\t\t\t" << std::to_string(throughput) <<"\t\t"<<fairnessIndex << "\n";
      	
      	std::cout <<  std::to_string(packetSize) << "\t\t\t" << (((i->second.rxBytes*8)/time)/1000)<<"\t\t"<<fairnessIndex << "\n";
  		Simulator::Destroy ();
	}
	return 0;

}