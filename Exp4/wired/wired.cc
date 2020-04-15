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

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WiredConnection") ;

int main( int argc , char ** argv )
{
	LogComponentEnable("WiredConnection" , LOG_INFO);

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
	std::string traceFileName = "traceWiredTcp" + tcpAgent + ".txt";

  	Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (traceFileName);
    *stream->GetStream () << "Using TCP Agent: Tcp" << tcpAgent << "\n";
    *stream->GetStream () <<"Packet Size \t\tThroughput \t\tFairness Index " << "\n";

	for( int index = 0 ; index<packetSizes.size() ; index++)
	{
		int packetSize = packetSizes[index];
        
        Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (packetSize));

		NS_LOG_INFO("Creating nodes");
		NodeContainer nodes;
		nodes.Create(4);

		PointToPointHelper hostToRouter;
		hostToRouter.SetDeviceAttribute("DataRate" , StringValue("100Mbps") );
		hostToRouter.SetChannelAttribute("Delay" , StringValue("20ms") );
        hostToRouter.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize("250000B")));  

	    PointToPointHelper routerToRouter;
		routerToRouter.SetDeviceAttribute("DataRate" , StringValue("10Mbps") );
		routerToRouter.SetChannelAttribute("Delay" , StringValue("50ms") );
        routerToRouter.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize("62500B")));  

	    NetDeviceContainer netDevices_1 , netDevices_2 , netDevices_3;
	    netDevices_1 = hostToRouter.Install( nodes.Get(0) , nodes.Get(1) );
	    netDevices_2 = routerToRouter.Install(nodes.Get(1) , nodes.Get(2) );
	    netDevices_3 = hostToRouter.Install(nodes.Get(2) , nodes.Get(3) );

	    InternetStackHelper internet;
	    internet.Install(nodes);

	    NS_LOG_INFO( "Assigning IP Addresses." );
  		Ipv4AddressHelper ipv4;
  		ipv4.SetBase( "10.1.1.0" , "255.255.255.0" );
	  	Ipv4InterfaceContainer interfaces_1 = ipv4.Assign ( netDevices_1 );

  		ipv4.SetBase( "10.1.2.0" , "255.255.255.0" );
	  	Ipv4InterfaceContainer interfaces_2 = ipv4.Assign ( netDevices_2 );

  		ipv4.SetBase( "10.1.3.0" , "255.255.255.0" );
	  	Ipv4InterfaceContainer interfaces_3 = ipv4.Assign ( netDevices_3 );

	  	ApplicationContainer sourceApps;
	  	ApplicationContainer sinkApps;
	    Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

	    uint16_t port = 10000 + index;

      	BulkSendHelper source ("ns3::TcpSocketFactory", InetSocketAddress (interfaces_3.GetAddress (1), port));
      	source.SetAttribute("SendSize" , UintegerValue(packetSize));
      	source.SetAttribute("MaxBytes" , UintegerValue(0));
      	sourceApps= (source.Install( nodes.Get(0) ) );

		PacketSinkHelper sink ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), port));
		sinkApps= (sink.Install(nodes.Get(3)));

		sinkApps.Start(Seconds(0));
		sinkApps.Stop(Seconds(5));
		sourceApps.Start (Seconds (0));
	  	sourceApps.Stop (Seconds (5));
 		
 		FlowMonitorHelper flowmon;
		Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

 		Simulator::Stop (Seconds (5));
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