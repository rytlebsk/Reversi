#include"User.h"

User::User()
{
}

User::~User()
{
}

User::User(int playerId) {
	id = playerId;
	name = findName(playerId);
	game = findGame(playerId);
}

