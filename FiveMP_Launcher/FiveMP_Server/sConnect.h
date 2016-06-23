class sNetCode {
public:
	unsigned char GetPacketIdentifier(RakNet::Packet * p);

	RakNet::RakPeerInterface *server;
	RakNet::RakNetStatistics *rss;
	RakNet::Packet* p;
	RakNet::SystemAddress clientID;
	RakNet::SocketDescriptor socketDescriptors[2];

	unsigned char packetIdentifier;

	void Initialize(void);
	void Connect(char *port, int maxplayers);
	void ReceivePacket();
};