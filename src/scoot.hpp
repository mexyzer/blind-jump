//========================================================================//
// Copyright (C) 2016 Evan Bowman                                         //
// Liscensed under GPL 3, see: <http://www.gnu.org/licenses/>.            //
//========================================================================//

#pragma once

#include "wall.hpp"
#include "effectsController.hpp"
#include "enemy.hpp"
#include "resourceHandler.hpp"
#include "RenderType.hpp"

class Scoot : public Enemy {
public:
	using HBox = framework::HitBox<12, 12, -6, -6>;
	Scoot(const sf::Texture &, const sf::Texture &, float, float);
	void update(float, float, const std::vector<wall> &, EffectGroup & ef, const sf::Time &);
	const sf::Sprite & getSprite() const;
	const sf::Sprite & getShadow() const;
	const HBox & getHitBox() const;
	
private:
	enum class State {
		drift1, drift2, shoot, recoil
	};
	mutable SpriteSheet<88, 161, 12, 12> spriteSheet;
	sf::Sprite shadow;
	float speedScale, hSpeed, vSpeed;
	State state;
	int32_t timer;
	void changeDir(float);
	void onDeath(EffectGroup &);
	HBox hitBox;
};
