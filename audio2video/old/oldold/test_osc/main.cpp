
#include <iostream>
#include <cstring>

#include "oscpack/osc/OscReceivedElements.h"
#include "oscpack/osc/OscPacketListener.h"
#include "oscpack/ip/UdpSocket.h"


#define PORT 12000

class ExamplePacketListener : public osc::OscPacketListener {
protected:

    virtual void ProcessMessage(const osc::ReceivedMessage& m, const IpEndpointName& remoteEndpoint) {
        (void) remoteEndpoint; // suppress unused parameter warning

        try {
            // example of parsing single messages. osc::OsckPacketListener
            // handles the bundle traversal.
            
            if (std::strcmp( m.AddressPattern(), "/create" )== 0) {
                // example #1 -- argument stream interface
                osc::ReceivedMessageArgumentStream args = m.ArgumentStream();
                float track;
                args >> track >> osc::EndMessage;
                
                std::cout << "received create " << track << "\n";
                
            }
        } catch(osc::Exception& e) {
            // any parsing errors such as unexpected argument types, or 
            // missing arguments get thrown as exceptions.
            std::cout << "error while parsing message: " << m.AddressPattern() << ": " << e.what() << "\n";
        }
    }
};


int main(int argc, char* argv[]) {
    (void) argc; // suppress unused parameter warnings
    (void) argv; // suppress unused parameter warnings

    ExamplePacketListener listener;
    //UdpListeningReceiveSocket s(IpEndpointName(IpEndpointName::ANY_ADDRESS, PORT), &listener);
    UdpListeningReceiveSocket *s;
    s= new UdpListeningReceiveSocket(IpEndpointName(IpEndpointName::ANY_ADDRESS, PORT), &listener);

    std::cout << "press ctrl-c to end\n";

    s->RunUntilSigInt();
    
    std::cout << "test\n";

    return 0;
}

