//========================================================================//
// Copyright (C) 2016 Evan Bowman                                         //
//                                                                        //
// This program is free software: you can redistribute it and/or modify   //
// it under the terms of the GNU General Public License as published by   //
// the Free Software Foundation, either version 3 of the License, or      //
// (at your option) any later version.                                    //
//                                                                        //
// This program is distributed in the hope that it will be useful,        //
// but WITHOUT ANY WARRANTY; without even the implied warranty of         //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          //
// GNU General Public License for more details.                           //
//                                                                        //
// You should have received a copy of the GNU General Public License      //
// along with this program.  If not, see <http://www.gnu.org/licenses/>.  //
//========================================================================//

#pragma once
#ifndef TEXTUREMANAGER_HPP
#define TEXTUREMANAGER_HPP

#include <SFML/Graphics.hpp>
#include <map>
#include <utility>
#include <array>
#include "ResourcePath.hpp"

class ResourceHandler {	
public:	
    enum class Texture {
		gameObjects,
		vignette,
		vignetteShadow,
		lamplight,
		introLevel,
		teleporterGlow,
		introWall,
		redglow,
		blueglow,
		fireExplosionGlow,
		whiteGlow,
		scootShadow
	};
	enum class Shader {
		color,
		blur,
		desaturate
	};
	enum class Font {
		cornerstone
	};
	enum class Image {
		soilTileset, grassSet1, grassSet2
	};
	
	int load();
	
	const sf::Texture & getTexture(ResourceHandler::Texture) const;
	const sf::Texture & getTexture(int) const;
	const sf::Font & getFont(ResourceHandler::Font) const;
	const sf::Image & getImage(ResourceHandler::Image) const;
	sf::Shader & getShader(ResourceHandler::Shader) const; // Exception: shader cannot be a constant reference
	
private:
	mutable std::map<int, sf::Texture> textures;
	mutable std::array<sf::Shader, 3> shaders;
	mutable std::array<sf::Font, 1> fonts;
	mutable std::array<sf::Image, 3> images;
};

// Many classes need to access the resource handler, and its accessors return only const references
extern ResourceHandler globalResourceHandler;

#endif
