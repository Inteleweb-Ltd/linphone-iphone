/*
 * remote-conference.cpp
 * Copyright (C) 2010-2017 Belledonne Communications SARL
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "participant-p.h"
#include "remote-conference-event-handler.h"
#include "remote-conference-p.h"
#include "xml/resource-lists.h"

// =============================================================================

using namespace std;

LINPHONE_BEGIN_NAMESPACE

RemoteConference::RemoteConference (LinphoneCore *core, const Address &myAddress, CallListener *listener)
	: Conference(*new RemoteConferencePrivate, core, myAddress, listener) {
	L_D();
	d->eventHandler.reset(new RemoteConferenceEventHandler(core, this));
}

RemoteConference::~RemoteConference () {
	L_D();
	d->eventHandler->unsubscribe();
}

// -----------------------------------------------------------------------------

void RemoteConference::addParticipant (const Address &addr, const CallSessionParams *params, bool hasMedia) {
	L_D();
	shared_ptr<Participant> participant = findParticipant(addr);
	if (participant)
		return;
	participant = make_shared<Participant>(addr);
	participant->getPrivate()->createSession(*this, params, hasMedia, this);
	d->participants.push_back(participant);
	if (!d->activeParticipant)
		d->activeParticipant = participant;
}

void RemoteConference::removeParticipant (const shared_ptr<const Participant> &participant) {
	L_D();
	for (const auto &p : d->participants) {
		if (participant->getAddress() == p->getAddress()) {
			d->participants.remove(p);
			return;
		}
	}
}

string RemoteConference::getResourceLists (const list<Address> &addresses) const {
	Xsd::ResourceLists::ResourceLists rl = Xsd::ResourceLists::ResourceLists();
	Xsd::ResourceLists::ListType l = Xsd::ResourceLists::ListType();
	for (const auto &addr : addresses) {
		Xsd::ResourceLists::EntryType entry = Xsd::ResourceLists::EntryType(addr.asStringUriOnly());
		if (!addr.getDisplayName().empty())
			entry.setDisplayName(Xsd::ResourceLists::DisplayName(addr.getDisplayName()));
		l.getEntry().push_back(entry);
	}
	rl.getList().push_back(l);

	Xsd::XmlSchema::NamespaceInfomap map;
	stringstream xmlBody;
	serializeResourceLists(xmlBody, rl, map);
	return xmlBody.str();
}

// -----------------------------------------------------------------------------

void RemoteConference::onConferenceCreated (const Address &addr) {}

void RemoteConference::onConferenceTerminated (const Address &addr) {
	L_D();
	d->eventHandler->unsubscribe();
}

void RemoteConference::onParticipantAdded (time_t tm, const Address &addr) {}

void RemoteConference::onParticipantRemoved (time_t tm, const Address &addr) {}

void RemoteConference::onParticipantSetAdmin (time_t tm, const Address &addr, bool isAdmin) {}

void RemoteConference::onSubjectChanged (time_t tm, const std::string &subject) {}

void RemoteConference::onParticipantDeviceAdded (time_t tm, const Address &addr, const Address &gruu) {}

void RemoteConference::onParticipantDeviceRemoved (time_t tm, const Address &addr, const Address &gruu) {}

LINPHONE_END_NAMESPACE
