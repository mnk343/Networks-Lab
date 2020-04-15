/*
	Networks Assignment 4 Application #2 Wired Connection 
	
	All traces can be found in the 'wired' folder
	All plots of the simulation can be found in the 'plots' folder.

	THE NETWORK TOPOLOGY IS AS FOLLOWS : 	
 
 *
 *  node2-----R1--------R2-----node3
 *        100    10Mbps    100
 *        Mbps    50ms     Mbps
 *        20ms             20ms
 *
*/

#include <string>
#include <vector>
#include <fstream>
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/network-module.h"
#include "ns3/packet-sink.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/drop-tail-queue.h"

// The ns3 project has been created in a namespace called 'ns3' 
using namespace ns3;

// We define the Component, this will be later used to diplay info messages on the terminal
NS_LOG_COMPONENT_DEFINE ("WiredConnection") ;

int main( int argc , char ** argv )
{
	// In order to display proper messages on the terminal, we enable the Component we defined above
	LogComponentEnable("WiredConnection" , LOG_INFO);

	std::string tcpAgent = "TcpWestwood";
	std::vector<int> packetSizes = {40, 44, 48, 52, 60, 250, 300, 552, 576, 628, 1420 ,1500};

	// Commandline arguments are used to take the TCP Agent from the user
	CommandLine cmd;
	cmd.AddValue("TcpAgent" , "Specifies the TCP Agent to be used, available options are TCP Westwood,TCP Veno and TCP Vegas" , tcpAgent);
	cmd.Parse(argc,argv);

	// Based on the input from the user, we set the TCP Agent appropriately. As given in the question 
	// the user has an option to choose among TCP Veno, TCP Vegas and TCP Westwood
	
	if( tcpAgent == "Westwood" )
    	Config::SetDefault("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVeno"));
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
	std::string traceFileName = "traceWiredTcp" + tcpAgent + ".txt";

	// This is the output stream, all the relevant data will be pur in this stream
  	Ptr<OutputStreamWrapper> stream = asciiTraceHelper.CreateFileStream (traceFileName);
    *stream->GetStream () << "Using TCP Agent: Tcp" << tcpAgent << "\n";
    *stream->GetStream () <<"Packet Size \t\tThroughput \t\tFairness Index " << "\n";

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

		// We use a topology helper object to do the low-level work required to put the link together
		// So according to our topology we will need 2 kinds of PointToPointHelpers
		// one to connect router to a host and one to connect both the routers
		// We do this next setting the values of the Delay and DataRate as given in the question
		// We also set the DroptailQueueSize according to the bandwidth delay product 

		PointToPointHelper hostToRouter;
		hostToRouter.SetDeviceAttribute("DataRate" , StringValue("100Mbps") );
		hostToRouter.SetChannelAttribute("Delay" , StringValue("20ms") );
        hostToRouter.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize("250000B")));  

	    PointToPointHelper routerToRouter;
		routerToRouter.SetDeviceAttribute("DataRate" , StringValue("10Mbps") );
		routerToRouter.SetChannelAttribute("Delay" , StringValue("50ms") );
        routerToRouter.SetQueue("ns3::DropTailQueue<Packet>", "MaxSize", QueueSizeValue (QueueSize("62500B")));  

        // After creating the PointToPointHelper, and giving the required attributes,we
        // now install the required network drivers which will interact with the channel
        // directly. So there are 3 kinds of network device drivers.

	    NetDeviceContainer netDevices_1 , netDevices_2 , netDevices_3;
	    netDevices_1 = hostToRouter.Install( nodes.Get(0) , nodes.Get(1) );
	    netDevices_2 = routerToRouter.Install(nodes.Get(1) , nodes.Get(2) );
	    netDevices_3 = hostToRouter.Install(nodes.Get(2) , nodes.Get(3) );

	    // We then initialize the internet stack in all the nodes.
	    InternetStackHelper internet;
	    internet.Install(nodes);

	    // Next, we assign IP Addresses to all kinds of network device drivers.
	    // We set the base and subnet and pass the NetDeviceContainer as an argument.
	    NS_LOG_INFO( "Assigning IP Addresses." );
  		Ipv4AddressHelper ipv4;
  		ipv4.SetBase( "10.1.1.0" , "255.255.255.0" );
	  	Ipv4InterfaceContainer interfaces_1 = ipv4.Assign ( netDevices_1 );

  		ipv4.SetBase( "10.1.2.0" , "255.255.255.0" );
	  	Ipv4InterfaceContainer interfaces_2 = ipv4.Assign ( netDevices_2 );

  		ipv4.SetBase( "10.1.3.0" , "255.255.255.0" );
	  	Ipv4InterfaceContainer interfaces_3 = ipv4.Assign ( netDevices_3 );

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

	    uint16_t port = 10000 + index;

	    // Now from the source side, we had 3 options :
	    // to make a custom application as done in fourth.cc of examples/tutorial
	    // to make a bulk application or to make a onoff application
	    // Since we didnt had to deal with callbacks in the assignment, we have used bulksendapplication
	    // which tries to send as much traffic as possible thus utilizing maximum bandwidth.
      	BulkSendHelper source ("ns3::TcpSocketFactory", InetSocketAddress (interfaces_3.GetAddress (1), port));
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