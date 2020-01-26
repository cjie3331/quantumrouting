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

#include "ns3/pointer.h"
#include "quantumrouting-helper.h"

namespace ns3
{
QuantumRoutingHelper::QuantumRoutingHelper ()
 : Ipv4RoutingHelper ()
{
  m_agentFactory.SetTypeId ("ns3::QuantumRouting");
}

QuantumRoutingHelper* 
QuantumRoutingHelper::Copy (void) const 
{
  return new QuantumRoutingHelper (*this);
}

Ptr<Ipv4RoutingProtocol> 
QuantumRoutingHelper::Create (Ptr<Node> node) const
{
  Ptr<QuantumRouting> agent = m_agentFactory.Create<QuantumRouting> ();
  node->AggregateObject (agent);
  return agent;
}

void 
QuantumRoutingHelper::Set (std::string name, const AttributeValue &value)
{
  m_agentFactory.Set (name, value);
}

}
