/*
 * libretroshare/src/chat/distributedchat.h
 *
 * Services for RetroShare.
 *
 * Copyright 2014 by Cyril Soler
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License Version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA.
 *
 * Please report all bugs and problems to "csoler@users.sourceforge.net".
 *
 */
#pragma once

#include <retroshare/rstypes.h>
#include <retroshare/rsmsgs.h>
#include <retroshare/rsservicecontrol.h>

typedef RsPeerId ChatLobbyVirtualPeerId ;

class RsItem ;
class p3HistoryMgr ;
class p3ServiceControl;
class RsChatLobbyItem ;
class RsChatLobbyListRequestItem ;
class RsChatLobbyListItem ;
class RsChatLobbyEventItem ;
class RsChatLobbyBouncingObject ;
class RsChatLobbyInviteItem ;
class RsChatLobbyMsgItem ;
class RsChatLobbyConnectChallengeItem ;
class RsChatLobbyUnsubscribeItem ;

class RsChatItem ;
class RsChatMsgItem ;

class DistributedChatService
{
	public:
		DistributedChatService(uint32_t service_type,p3ServiceControl *sc,p3HistoryMgr *hm) ;

		virtual ~DistributedChatService() {}

		void flush() ;

		// Interface part to communicate with
		//
		bool getVirtualPeerId(const ChatLobbyId& lobby_id, RsPeerId& virtual_peer_id) ;
		bool isLobbyId(const RsPeerId& virtual_peer_id, ChatLobbyId& lobby_id) ;
		void getChatLobbyList(std::list<ChatLobbyInfo, std::allocator<ChatLobbyInfo> >& cl_infos) ;
		bool acceptLobbyInvite(const ChatLobbyId& id) ;
		void denyLobbyInvite(const ChatLobbyId& id) ;
		void getPendingChatLobbyInvites(std::list<ChatLobbyInvite>& invites) ;
		void invitePeerToLobby(const ChatLobbyId&, const RsPeerId& peer_id,bool connexion_challenge = false) ;
		void unsubscribeChatLobby(const ChatLobbyId& lobby_id) ;
		bool setNickNameForChatLobby(const ChatLobbyId& lobby_id,const std::string& nick) ;
		bool getNickNameForChatLobby(const ChatLobbyId& lobby_id,std::string& nick) ;
		bool setDefaultNickNameForChatLobby(const std::string& nick) ;
		bool getDefaultNickNameForChatLobby(std::string& nick) ;
		void setLobbyAutoSubscribe(const ChatLobbyId& lobby_id, const bool autoSubscribe);
		bool getLobbyAutoSubscribe(const ChatLobbyId& lobby_id);
		void sendLobbyStatusString(const ChatLobbyId& id,const std::string& status_string) ;

		ChatLobbyId createChatLobby(const std::string& lobby_name,const std::string& lobby_topic, const std::list<RsPeerId>& invited_friends,uint32_t privacy_type) ;

		void getListOfNearbyChatLobbies(std::vector<VisibleChatLobbyRecord>& public_lobbies) ;
		bool joinVisibleChatLobby(const ChatLobbyId& id) ;

	protected:
		bool handleRecvItem(RsChatItem *) ;

		virtual void sendChatItem(RsChatItem *) =0 ;
		virtual void locked_storeIncomingMsg(RsChatMsgItem *) =0 ;
		virtual void triggerConfigSave() =0;

		void addToSaveList(std::list<RsItem*>& list) const ;
        bool processLoadListItem(const RsItem *item) ;

        bool locked_checkAndRebuildPartialLobbyMessage(RsChatLobbyMsgItem *) ;
        void checkSizeAndSendLobbyMessage(RsChatLobbyMsgItem *) ;

        bool sendLobbyChat(const RsPeerId &id, const std::string&, const ChatLobbyId&) ;
        bool handleRecvChatLobbyMsgItem(RsChatMsgItem *item) ;

    private:
		/// make some statistics about time shifts, to prevent various issues. 
		void addTimeShiftStatistics(int shift) ;

		void handleRecvChatLobbyListRequest(RsChatLobbyListRequestItem *item) ;
		void handleRecvChatLobbyList(RsChatLobbyListItem *item) ;
        void handleRecvChatLobbyEventItem(RsChatLobbyEventItem *item) ;


		/// Checks that the lobby object is not flooding a lobby.
		bool locked_bouncingObjectCheck(RsChatLobbyBouncingObject *obj,const RsPeerId& peer_id,uint32_t lobby_count) ;

		/// receive and handle chat lobby item
		bool recvLobbyChat(RsChatLobbyMsgItem*,const RsPeerId& src_peer_id) ;
		void handleRecvLobbyInvite(RsChatLobbyInviteItem*) ;
		void checkAndRedirectMsgToLobby(RsChatMsgItem*) ;
		void handleConnectionChallenge(RsChatLobbyConnectChallengeItem *item) ;
		void sendConnectionChallenge(ChatLobbyId id) ;
		void handleFriendUnsubscribeLobby(RsChatLobbyUnsubscribeItem*) ;
		void cleanLobbyCaches() ;
		bool bounceLobbyObject(RsChatLobbyBouncingObject *obj, const RsPeerId& peer_id) ;

		void sendLobbyStatusItem(const ChatLobbyId&, int type, const std::string& status_string) ;
		void sendLobbyStatusPeerLiving(const ChatLobbyId& lobby_id) ;
		void sendLobbyStatusPeerChangedNickname(const ChatLobbyId& lobby_id, const std::string& newnick) ;

		void sendLobbyStatusNewPeer(const ChatLobbyId& lobby_id) ;
		void sendLobbyStatusKeepAlive(const ChatLobbyId&) ;

		bool locked_initLobbyBouncableObject(const ChatLobbyId& id,RsChatLobbyBouncingObject&) ;
        void locked_printDebugInfo() const ;

		static ChatLobbyVirtualPeerId makeVirtualPeerId(ChatLobbyId) ;
		static uint64_t makeConnexionChallengeCode(const RsPeerId& peer_id,ChatLobbyId lobby_id,ChatLobbyMsgId msg_id) ;

		class ChatLobbyEntry: public ChatLobbyInfo
		{
			public:
				std::map<ChatLobbyMsgId,time_t> msg_cache ;
				RsPeerId virtual_peer_id ;
				int connexion_challenge_count ;
				time_t last_connexion_challenge_time ;
				time_t last_keep_alive_packet_time ;
				std::set<RsPeerId> previously_known_peers ;
				uint32_t flags ;
		};

		std::map<ChatLobbyId,ChatLobbyEntry> _chat_lobbys ;
		std::map<ChatLobbyId,ChatLobbyInvite> _lobby_invites_queue ;
		std::map<ChatLobbyId,VisibleChatLobbyRecord> _visible_lobbies ;
		std::map<ChatLobbyVirtualPeerId,ChatLobbyId> _lobby_ids ;
		std::map<ChatLobbyId,ChatLobbyFlags> _known_lobbies_flags ;	// flags for all lobbies, including the ones that are not known. So we can't
        std::map<ChatLobbyId,std::vector<RsChatLobbyMsgItem*> > _pendingPartialLobbyMessages ;																	// store them in _chat_lobbies (subscribed lobbies) nor _visible_lobbies.
																				// Known flags:
																				// 		RS_CHAT_LOBBY_FLAGS_AUTO_SUBSCRIBE

		float _time_shift_average ;
		time_t last_lobby_challenge_time ; 					// prevents bruteforce attack
		time_t last_visible_lobby_info_request_time ;	// allows to ask for updates
		bool _should_reset_lobby_counts ;
		std::string _default_nick_name ;

		uint32_t mServType ;
		RsMutex mDistributedChatMtx ;

		p3ServiceControl *mServControl; 
		p3HistoryMgr *mHistMgr; 
};