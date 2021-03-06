/*
 * Copyright (c) 2014, 2015 Cisco Systems, Inc. and others. All rights reserved.
 *
 * This program and the accompanying materials are made available under the
 * terms of the Eclipse Public License v1.0 which accompanies this distribution,
 * and is available at http://www.eclipse.org/legal/epl-v10.html
 */

package org.opendaylight.snbi.southplugin;

import java.net.InetAddress;
import java.net.NetworkInterface;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public abstract class SnbiNodeStateCommonEventHandlers implements ISnbiNodeState {
    protected SnbiNode node = null;
    private static final Logger log = LoggerFactory.getLogger(SnbiNodeStateCommonEventHandlers.class);


    public SnbiNodeStateCommonEventHandlers (SnbiNode node) {
        this.node = node;
    }
    
    @Override
	public SnbiNodeState handleNodeCertReqPktEvent (SnbiPkt pkt) {
        log.debug("[node:"+node.getUDI()+"] Handle NI Cert Req pkt event: "+pkt.getUDITLV());
        sendNodeCertResponseMsg(pkt.getSrcIP(), pkt.getIngressInterface());
        return SnbiNodeState.SNBI_NODE_STATE_NO_CHANGE;
    }

    private void sendNodeCertResponseMsg (InetAddress dstIP, NetworkInterface egressIntf) {
        SnbiPkt pkt = new SnbiPkt (SnbiProtocolType.SNBI_PROTOCOL_BOOTSTRAP, SnbiMsgType.SNBI_MSG_NODE_CERT_RESP);
        pkt.setUDITLV(node.getRegistrar().getNodeself().getUDI());
        pkt.setDstIP(dstIP);
        pkt.setSrcIP(node.getRegistrar().getNodeself().getNodeAddress());
        pkt.setEgressInterface(egressIntf);

        if (node.getRegistrar().getNodeself().isBootStrapped()) {
            pkt.setDomainCertTLV(node.getRegistrar().getNodeself().getCertificate());
        }
        SnbiMessagingInfra.getInstance().packetSend(pkt);
    }

    @Override
	public SnbiNodeState handleNodeConnectPktEvent (SnbiPkt pkt) {
        log.debug("[node:"+node.getUDI()+"] Handle Nbr Connect pkt Event: "+pkt.getUDITLV());
        SnbiNodeState nextState = bsInviteValidateGetNextState();
        return nextState;
    }

    @Override
	public SnbiNodeState handleNodeBSReqPktEvent (SnbiPkt pkt) {
        log.debug("[node:"+node.getUDI()+"] Handle BSReq Pkt Event: "+pkt.getUDITLV());
        SnbiNodeState nextState = bsReqValidateGetNextState();
        if (nextState == SnbiNodeState.SNBI_NODE_STATE_BOOTSTRAP) {
            node.SetCertificate(CertManager.getInstance().generateX509Certificate(pkt.getPKCS10CSRTLV(), null));
            log.debug("Certificate signed "+node.getCertificate().toString());
            node.setBootStrapped(true);
        }
        return (nextState);
    }

    private SnbiNodeState bsReqValidateGetNextState () {
        if (node.getRegistrar().validateNode(node)) {
            return SnbiNodeState.SNBI_NODE_STATE_BOOTSTRAP;
        } else {
            return SnbiNodeState.SNBI_NODE_STATE_BOOTSTRAP_IGNORE;
        }
    }

    private SnbiNodeState bsInviteValidateGetNextState () {
        if (node.getRegistrar().validateNode(node)) {
            log.debug("[node:"+node.getUDI()+"] BS Invite node");
            return SnbiNodeState.SNBI_NODE_BS_INVITE;
        } else {
            return SnbiNodeState.SNBI_NODE_BS_REJECTED;
        }
    }
}
