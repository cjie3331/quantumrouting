/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2010 University of Arizona
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as 
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Author: Junseok Kim <junseok@email.arizona.edu> <engr.arizona.edu/~junseok>
 */

#include "ns3/log.h"
#include "ns3/ipv4.h"
#include "ns3/ipv4-route.h"
#include "ns3/pointer.h"
#include "ns3/double.h"
#include "ns3/ipv4-static-routing.h"
#include "quantumrouting-table.h"
#include "quantumrouting.h"

NS_LOG_COMPONENT_DEFINE ("QuantumRouting");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED (QuantumRouting);

TypeId
QuantumRouting::GetTypeId (void)
{
  static TypeId tid = TypeId ("ns3::QuantumRouting")
    .SetParent<Ipv4RoutingProtocol> ()
    .AddConstructor<QuantumRouting> ()
    .AddAttribute ("RoutingTable", "Pointer to Routing Table.",
                   PointerValue (),
                   MakePointerAccessor (&QuantumRouting::SetRtable),
                   MakePointerChecker<QuantumRoutingTable> ())
    ;
  return tid;
}


QuantumRouting::QuantumRouting () 
{
  NS_LOG_FUNCTION_NOARGS ();
}

QuantumRouting::~QuantumRouting () 
{
  NS_LOG_FUNCTION_NOARGS ();
}

Ptr<Ipv4Route>
QuantumRouting::RouteOutput (Ptr<Packet> p, const Ipv4Header &header, Ptr<NetDevice> oif, enum Socket::SocketErrno &sockerr)
{
  int flowID = p->getFlowID();
  std::cout<<"Output-flowID is :"<<flowID<<endl;
  Ipv4Address destaddr=header.GetDestination ();
  std::vector<SpNodeEntry>::iterator it = m_rtable->m_nodeTable.begin ();
  int i=0;
      for (; it != m_rtable->m_nodeTable.end ();++it)
        {
          if (it->addr == destaddr)
            {
              break;
            }
          i++;
        }
  if(i!=0){p->setFlowID(i);}
  flowID=p->getFlowID();

  Ipv4Address relay = m_rtable->LookupRoute (m_address, header.GetDestination (),flowID);
  NS_LOG_FUNCTION (this << header.GetSource () << "->" << relay << "->" << header.GetDestination ());
  NS_LOG_INFO ("Relay to " << relay);
  if (m_address == relay)
    {
      NS_LOG_DEBUG ("Can't find route!!");
    }
  
  Ptr<Ipv4Route> route = Create<Ipv4Route> ();
  route->SetGateway (relay);
  route->SetSource (m_address);
  route->SetDestination (header.GetDestination ());
  route->SetOutputDevice (m_ipv4->GetNetDevice (m_ifaceId));
  
  sockerr = Socket::ERROR_NOTERROR;
  
  return route;
}

bool 
QuantumRouting::RouteInput (Ptr<const Packet> p, const Ipv4Header &header, Ptr<const NetDevice> idev, 
                             UnicastForwardCallback ucb, MulticastForwardCallback mcb, 
                             LocalDeliverCallback lcb, ErrorCallback ecb)
{
  NS_LOG_FUNCTION (header.GetDestination ());
  int flowID=p->getFlowID();
  std::cout<<"Input-flowID is :"<<flowID<<endl;
  if (header.GetDestination () == m_address)
    {
      NS_LOG_DEBUG ("I'm the destination");
      lcb (p, header, m_ipv4->GetInterfaceForDevice (idev));
      return true;
    }
  else if (header.GetDestination () == m_broadcast)
    {
      NS_LOG_DEBUG ("It's broadcast");
      return true;
    }
  else
    {
      Ipv4Address relay = m_rtable->LookupRoute (m_address, header.GetDestination (),flowID);
      NS_LOG_FUNCTION (this << m_address << "->" << relay << "->" << header.GetDestination ());
      NS_LOG_DEBUG ("Relay to " << relay);
      if (m_address == relay)
        {
          NS_LOG_DEBUG ("Can't find a route!!");
        }
      Ptr<Ipv4Route> route = Create<Ipv4Route> ();
      route->SetGateway (relay);
      route->SetSource (header.GetSource ());
      route->SetDestination (header.GetDestination ());
      route->SetOutputDevice (m_ipv4->GetNetDevice (m_ifaceId));
      ucb (route, p, header);
      return true;
    }
  return false;
}

void 
QuantumRouting::NotifyInterfaceUp (uint32_t interface)
{
  NS_LOG_FUNCTION (this << interface);
}
void 
QuantumRouting::NotifyInterfaceDown (uint32_t interface)
{
  NS_LOG_FUNCTION (this << interface);
}
void 
QuantumRouting::NotifyAddAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION(this << interface << address << m_rtable);
  m_ifaceId = interface;
  m_address = address.GetLocal ();
  m_broadcast = address.GetBroadcast ();
}
void 
QuantumRouting::NotifyRemoveAddress (uint32_t interface, Ipv4InterfaceAddress address)
{
  NS_LOG_FUNCTION(this << interface << address);
}
void 
QuantumRouting::SetIpv4 (Ptr<Ipv4> ipv4)
{
  NS_LOG_FUNCTION(this << ipv4);
  m_ipv4 = ipv4;
}
void
QuantumRouting::PrintRoutingTable (Ptr<OutputStreamWrapper> stream,Time::Unit unit) const
{
  *stream->GetStream () << "Node: " << m_ipv4->GetObject<Node> ()->GetId ();
  m_rtable->Print (stream);
}
void
QuantumRouting::SetRtable (Ptr<QuantumRoutingTable> p)
{
  NS_LOG_FUNCTION(p);
  m_rtable = p;
}

} // namespace ns3


