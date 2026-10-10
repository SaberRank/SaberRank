#include "Features/Players/Domain/Player.hpp"
namespace SnoreSaber::Data
{
    Player::Player(std::string _id)
    {
        id = _id;
        pp = 0.0;
        rank = 0;
        countryRank = 0;
    }

    Player::Player(){};
} // namespace SnoreSaber::Data
