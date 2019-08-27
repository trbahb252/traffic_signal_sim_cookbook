/////////////////////////
//NTCIP 1202 Get/Set with SNMPSHARPNET DLL
//author: Pengfei (Taylor) Li of University of Texas Arlington
//Version 1.1
//Date: July--2019
//This program can either communicate with a real NTCIP_compliant traffic signal controller or the software-in-the-loop Econolite ASC/3
//the users will need set the IP addresses of (software) controller and the computer hosting this program withini the same subnetwork
//If this prorgram and VISSIM SILS is on the same machine. The IP address in the SILS ASC/3 must be 127.0.0.1; gateway is also 127.0.0.1; subnet mask 255.255.255.0
////////////////////////////////


using System;
using SnmpSharpNet;
using System.Net;

namespace ntcip
{
    class Program
    {
        static void Main(string[] args)
        {
            // SNMP community name
            OctetString community = new OctetString("public");

            // Define agent parameters class
            AgentParameters param = new AgentParameters(community);
            // Set SNMP version to 1 (or 2)
            param.Version = SnmpVersion.Ver1;
            // Construct the agent address object
            //IpAddress agent = new IpAddress("127.0.0.1"); //Taylor: Change the 127.0.0.1 to the IP address of your own controller's
            IpAddress agent = new IpAddress("192.168.2.109"); //Taylor: Change the 127.0.0.1 to the IP address of your own controller's
            // Construct target
            UdpTarget target = new UdpTarget((IPAddress)agent,501,2000,0); //Taylor: 503 should be the same NTCIP port in your controller's network setting. You may change to other port numbers (but be consistent with your controller) Other numbers do not matter

            // Pdu class used for all relevant requests
            Pdu pdu1 = new Pdu(PduType.Get);
            Pdu pdu2 = new Pdu(PduType.Get);
            Pdu pdu3 = new Pdu(PduType.Get);
            Pdu pdu4 = new Pdu(PduType.Get);
            Pdu pdu5 = new Pdu(PduType.Get);
            Pdu pdu6 = new Pdu(PduType.Get);
            Pdu pdu7 = new Pdu(PduType.Get);
            Pdu pdu8 = new Pdu(PduType.Get);
            Pdu pdu9 = new Pdu(PduType.Get);
            Pdu pdu10 = new Pdu(PduType.Get);
            Pdu pdu11 = new Pdu(PduType.Get);
            Pdu pdu12 = new Pdu(PduType.Get);
            Pdu pdu13 = new Pdu(PduType.Get);
            Pdu pdu14 = new Pdu(PduType.Get);
            Pdu pdu15 = new Pdu(PduType.Get);
            Pdu pdu16 = new Pdu(PduType.Get);

            //pdu class for set //taylor: add in May 2019
            Pdu pdu17 = new Pdu(PduType.Set);
            Pdu pdu18 = new Pdu(PduType.Set);
            Pdu pdu19 = new Pdu(PduType.Set);
            Pdu pdu20 = new Pdu(PduType.Set);
            Pdu pdu21 = new Pdu(PduType.Set);
            Pdu pdu22 = new Pdu(PduType.Set);


            pdu1.VbList.Add("1.3.6.1.4.1.1206.4.2.1.1.4.1.4.1"); //Taylor: green status of Phase 1-8;it will be a decimal integer. you need to convert it a 9-bit binary. for each bit, 1 is current green 0 is not green
            pdu2.VbList.Add("1.3.6.1.4.1.1206.4.2.1.1.4.1.4.2"); //Taylor: Phase 9-16; if not activated, then the returned value is zero
            pdu3.VbList.Add("1.3.6.1.4.1.1206.4.2.1.1.4.1.3.1"); //Taylor: yellow status of Phase 1-8; convert the returned decimal integer to binary. if a bit is 1, then currently that phase is yellow. otherwise is not yellow
            pdu4.VbList.Add("1.3.6.1.4.1.1206.4.2.1.1.4.1.3.2"); //Taylor: yellow phase 9-16; if the phase is no activated, then return value is zero
            //Taylor: no need to poll red since if neither green nor yellow, then it's red            

            /////////////////////Ped phase status
            pdu5.VbList.Add("1.3.6.1.4.1.1206.4.2.1.1.4.1.7.1"); //Taylor: walk status of Ped Phase 1-8; convert the returned decimal integer to binary. if a bit is 1, then currently that phase is yellow. otherwise is not yellow
            pdu6.VbList.Add("1.3.6.1.4.1.1206.4.2.1.1.4.1.7.2"); //Taylor: walk status of Ped Phase 9-16; convert the returned decimal integer to binary. if a bit is 1, then currently that phase is yellow. otherwise is not yellow
            pdu7.VbList.Add("1.3.6.1.4.1.1206.4.2.1.1.4.1.6.1"); //Taylor: ped clearance status of Ped Phase 1-8; convert the returned decimal integer to binary. if a bit is 1, then currently that phase is yellow. otherwise is not yellow
            pdu8.VbList.Add("1.3.6.1.4.1.1206.4.2.1.1.4.1.6.2"); //Taylor: ped clearance of Ped Phase 9-16; convert the returned decimal integer to binary. if a bit is 1, then currently that phase is yellow. otherwise is not yellow
            //if a ped phase is neither walk nor ped clearance. then it's solid "don't walk"
            
            //Poll dets status
            pdu9.VbList.Add("1.3.6.1.4.1.1206.4.2.1.2.4.1.2.1"); //Taylor: det status of channel 1-8; returned a decimal integer. converted to a binary first
            pdu10.VbList.Add("1.3.6.1.4.1.1206.4.2.1.2.4.1.2.2"); //Taylor: det status of channel 9-16; returned a decimal integer. converted to a binary first
            pdu11.VbList.Add("1.3.6.1.4.1.1206.4.2.1.2.4.1.2.3"); //Taylor: det status of channel 17-24; returned a decimal integer. converted to a binary first
            pdu12.VbList.Add("1.3.6.1.4.1.1206.4.2.1.2.4.1.2.4"); //Taylor: det status of channel 25-32; returned a decimal integer. converted to a binary first
            pdu13.VbList.Add("1.3.6.1.4.1.1206.4.2.1.2.4.1.2.5"); //Taylor: det status of channel 33-40; returned a decimal integer. converted to a binary first
            pdu14.VbList.Add("1.3.6.1.4.1.1206.4.2.1.2.4.1.2.6"); //Taylor: det status of channel 41-48; returned a decimal integer. converted to a binary first
            pdu15.VbList.Add("1.3.6.1.4.1.1206.4.2.1.2.4.1.2.7"); //Taylor: det status of channel 49-56; returned a decimal integer. converted to a binary first
            pdu16.VbList.Add("1.3.6.1.4.1.1206.4.2.1.2.4.1.2.8"); //Taylor: det status of channel 57-64; returned a decimal integer. converted to a binary first

            //Phase controls //
            pdu17.VbList.Add(new Oid("1.3.6.1.4.1.1206.4.2.1.1.5.1.2.1"), new Integer32(255)); //Taylor: phase 1~8 omit (if binary, from right to left: phase 1-8, but you will need to input the corresponding decimal). e.g., if you wish to omit 3 and 7. the binary value will be 01000100 and the decimal value is 68. replace the last digit in the 1.3.6.xxxx.2.1 with 2 if you wish to omit 9~16
            pdu18.VbList.Add(new Oid("1.3.6.1.4.1.1206.4.2.1.1.5.1.3.1"), new Integer32(68)); //Taylor: Ped 1-8 omits
            pdu19.VbList.Add(new Oid("1.3.6.1.4.1.1206.4.2.1.1.5.1.4.1"), new Integer32(68)); //Taylor: Phase 1-8 hold . 1.3....4."1". can be relaced with 2 for phase 9~16
            pdu20.VbList.Add(new Oid("1.3.6.1.4.1.1206.4.2.1.1.5.1.5.1"), new Integer32(68)); //taylor place or lift phase 1-8 force-off (if you wish to force phase 1 and 5, the binary will be 00010001 and the decmial is 17
            pdu21.VbList.Add(new Oid("1.3.6.1.4.1.1206.4.2.1.1.5.1.6.1"), new Integer32(68)); //Taylor: place (1) or lift (0)phase 1-8 vehicle call
            pdu21.VbList.Add(new Oid("1.3.6.1.4.1.1206.4.2.1.1.5.1.7.1"), new Integer32(68)); //Taylor: place or lift ped call


            // Taylor: Just use the green 1-8 and yellow 1-8 and det 1-8 as illustration. Please extend to others PDU as defined above if needed
            SnmpV1Packet result1 = (SnmpV1Packet)target.Request(pdu1, param);
            // green status of phase 1-8
            if (result1 != null)
            {
                // ErrorStatus other then 0 is an error returned by 
                // the Agent - see SnmpConstants for error definitions
                if (result1.Pdu.ErrorStatus != 0)
                {
                    // agent reported an error with the request
                    Console.WriteLine("Error in SNMP reply. Error {0} index {1}",
                        result1.Pdu.ErrorStatus,
                        result1.Pdu.ErrorIndex);
                }
                else
                {
                    // Reply variables are returned in the same order as they were added
                    //  to the VbList
                    String green1to8 = result1.Pdu.VbList[0].Value.ToString();//Taylor: Using https://www.mathsisfun.com/binary-decimal-hexadecimal-converter.html  to convert decimal to binary (from right to left, phase 1 to 8). You may need a function to do this in the code
                }
            }
            // yellow status of phase 1-8
            SnmpV1Packet result3 = (SnmpV1Packet)target.Request(pdu3, param);
            if (result3 != null)
            {
                // ErrorStatus other then 0 is an error returned by 
                // the Agent - see SnmpConstants for error definitions
                if (result3.Pdu.ErrorStatus != 0)
                {
                    // agent reported an error with the request
                    Console.WriteLine("Error in SNMP reply. Error {0} index {1}",
                        result3.Pdu.ErrorStatus,
                        result3.Pdu.ErrorIndex);
                }
                else
                {
                    // Reply variables are returned in the same order as they were added
                    //  to the VbList
                    String yellow1to8 = result3.Pdu.VbList[0].Value.ToString();
                }
            }
            // Status of det 1-8
            SnmpV1Packet result9 = (SnmpV1Packet)target.Request(pdu9, param);
            if (result9 != null)
            {
                // ErrorStatus other then 0 is an error returned by 
                // the Agent - see SnmpConstants for error definitions
                if (result9.Pdu.ErrorStatus != 0)
                {
                    // agent reported an error with the request
                    Console.WriteLine("Error in SNMP reply. Error {0} index {1}",
                        result9.Pdu.ErrorStatus,
                        result9.Pdu.ErrorIndex);
                }
                else
                {
                    // Reply variables are returned in the same order as they were added
                    //  to the VbList
                    String det1to8 = result9.Pdu.VbList[0].Value.ToString();
                }
            }
            else
            {
                Console.WriteLine("No response received from SNMP agent.");
            }

            //Set (write) example: phase 1-8 omit
            //SnmpV1Packet result17 = (SnmpV1Packet)target.Request(pdu17, param);
            SnmpV1Packet result17 = target.Request(pdu17, param) as SnmpV1Packet;// Taylor: I set the omit value 255 (1111111). it means all 8 phases are omit and you should be able to see this status on your front panel
            //                                                                                                            ^ (from left to right means the status of phase 8, 7,...,2,1)
            //If you wish to 
            if (result17 != null)
            {
                // ErrorStatus other then 0 is an error returned by 
                // the Agent - see SnmpConstants for error definitions
                if (result17.Pdu.ErrorStatus != 0)
                {
                    // agent reported an error with the request
                    Console.WriteLine("Error in SNMP reply. Error {0} index {1}",result17.Pdu.ErrorStatus,result17.Pdu.ErrorIndex);
                }
                else
                {
                    // Reply variables are returned in the same order as they were added
                    //  to the VbList
                    String phaseomit1to8 = result17.Pdu.VbList[0].Value.ToString();
                }
            }
            else
            {
                Console.WriteLine("No response received from SNMP agent.");
            }

            target.Close();
        }
    }
}
