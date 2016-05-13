//
//  mapInit.cpp
//  Blind Jump
//
//  Created by Evan Bowman on 10/15/15.
//  Copyright © 2015 Evan Bowman. All rights reserved.
//

#include "gameMap.hpp"
#include "mappingFunctions.hpp"
#include "initMapVectors.hpp"
#include <iostream>
#include <cmath>
#include "ResourcePath.hpp"
#include "enemyPlacementFn.hpp"
#include "lightingMap.h"
#include "pillarPlacement.h"
#include "math.h"
#include "enemyCreationFunctions.hpp"

// Define some macros to make the code easier to read
#define WHITE_TARGET 2
#define RED_TARGET 1
#define BLUE_TARGET 3
#define CRIMSON_TARGET 4
#define ARCTIC_TARGET 5
#define EFFECT_OBJECT 10

// Define a function to control the z-order sorting
struct sortKey {
    inline bool operator() (const std::tuple<sf::Sprite, float, int> arg1, const std::tuple<sf::Sprite, float, int> arg2) {
        return (std::get<1>(arg1) < std::get<1>(arg2));
    }
};

GameMap::GameMap(float windowWidth, float windowHeight, sf::Texture* inptxtr, InputController* input, FontController* pFonts) {
    //Make the background controller draw concenteric with the center of the view
    bkg.setPosition((tiles.posX / 2) + 226, tiles.posY / 2);
    
    // Set the size of the target render texture so that they'll fill the screen
    target.create(windowWidth, windowHeight);
    finalPass.create(windowWidth, windowHeight);
    finalPass.setSmooth(true);
    
    // Store a pointer to the input controller in main()
    pInput = input;
    
    worldView.setSize(windowWidth, windowHeight);
    worldView.setCenter(windowWidth / 2, windowHeight / 2);
    hudView.setSize(windowWidth, windowHeight);
    hudView.setCenter(windowWidth / 2, windowHeight / 2);
    
    //Store the inputs for later
    windowW = windowWidth;
    windowH = windowHeight;
    
    // Don't compute the blur effect when the ui menu is not opening or closing, use the cached version
    // This variable allows for that
    computeBlur = true;
    
    // Now call a function to procedurally distribute items
    initLoot(itemArray);
    
    // Initial teleporter condition to 0, meaning the player is not standing on a teleporter
    teleporterCond = 0;
    
    // Tell the background controller how big the window is so it doesn't draw out of bounds
    bkg.giveWindowSize(windowW, windowH);
    
    // Tell the UI where the center of the window is
    UI.setPosition(windowW / 2, windowH/ 2);
    
    // Store the pointer to the font controller in main() as one of the object's datefields
    this->pFonts = pFonts;
    
    // Set up the shaders
    colorShader.loadFromFile(resourcePath() + "color.frag", sf::Shader::Fragment);
    colorShader.setParameter("texture", sf::Shader::CurrentTexture);
    blurShader.loadFromFile(resourcePath() + "blur.frag", sf::Shader::Fragment);
    blurShader.setParameter("texture", sf::Shader::CurrentTexture);
    //Initialize the starting level to 1
    level = 0;
    
    // Setup the lighting map
    lightingMap.create(windowWidth, windowHeight);
    shadowShape.setFillColor(sf::Color(190, 190, 210, 255));
    sf::Vector2f v;
    v.x = windowWidth;
    v.y = windowHeight;
    shadowShape.setSize(v);
    
    //Initialize the world type
    worldType = rand() % 2;
    
    transitioning = false;
    transitionDelay = 320;
    
    dispEntryBeam = false;
    transitionIn = false;
    
    // Each object that isn't an effect or passed through a shader gets darkened according to the ambient conditions of the current tileset
    objectShadeColor = sf::Color(190, 190, 210, 255);
    
    titleTxtr.loadFromFile(resourcePath() + "title.png");
    titleSpr.setTexture(titleTxtr);
    titleSpr.setPosition(windowW / 2 - 109, windowH / 5 - 15);
    
    vignetteSprite.setTexture(*inptxtr);
    vignetteSprite.setScale(windowWidth/450, windowHeight/450);
    
    //Put the player in the center of the view
    player.setPosition(windowWidth / 2 - 16, windowHeight / 2);
    
    //Let the tile controller know where player is
    tiles.setPosition((windowWidth / 2) - 16, (windowHeight / 2));
    tiles.setWindowSize(windowWidth, windowHeight);
    
    /*  Completely non-general code only used by the intro level */
    details.addLamplight(tiles, tiles.posX - 180, tiles.posY + 200, 5, 6, windowWidth, windowHeight);
    details.addLamplight(tiles, tiles.posX - 180, tiles.posY + 200, 5, 0, windowWidth, windowHeight);
    details.addLamplight(tiles, tiles.posX - 170, tiles.posY + 200, 11, 11, windowWidth, windowHeight);
    details.addLamplight(tiles, tiles.posX - 180, tiles.posY + 200, 10, -9, windowWidth, windowHeight);
    details.addDoor(tiles.posX - 192, tiles.posY + 301, 6, 0, windowW, windowH);
    details.addPod(tiles.posX, tiles.posY + 33, 3, 17, pFonts);
    tiles.teleporterLocation.x = 8;
    tiles.teleporterLocation.y = -7;
    for (auto it = global_levelZeroWalls.begin(); it != global_levelZeroWalls.end(); ++it) {
        wall w;
        w.setXinit(it->first);
        w.setYinit(it->second);
        tiles.walls.push_back(w);
    }
    
    en.setWindowSize(windowWidth, windowHeight);
    
    //Initialize the fonts
    pFonts->setWaypointText(level);
    
    details.addTeleporter(tiles, tiles.posX - 178, tiles.posY + 284, windowW, windowH, pFonts);
    
    // initialize the rectangle shape for the teleporter beam effect
    sf::Vector2f v1(2, 1);
    teleporterBeam.setPosition(windowWidth/2 - 1.5, windowHeight/2 + 36);
    teleporterBeam.setSize(v1);
    teleporterBeam.setFillColor(sf::Color(114, 255, 229, 6));

    // Initialize the rectangle shape for the teleporter entry beam
    sf::Vector2f v2(4, 8);
    entryBeam.setPosition(windowW/2 - 2, -68);
    entryBeam.setSize(v2);
    entryBeam.setFillColor(sf::Color(104, 255, 229, 180));
    
    bkg.setBkg(0);
    
    beamExpanding = false;
    animationBegin = false;
    dispEntryBeam = false;
    
    sndCtrl.playMusic(0);
    beamGlowTxr.loadFromFile(resourcePath() + "teleporterBeamGlow.png");
    beamGlowSpr.setTexture(beamGlowTxr);
    beamGlowSpr.setPosition(windowWidth / 2 - 200, windowHeight / 2 - 200 + 30);
    beamGlowSpr.setColor(sf::Color(0, 0, 0, 255));
    
    transitionShape.setSize(sf::Vector2f(windowW, windowH));
    transitionShape.setFillColor(sf::Color(0, 0, 0, 0));
    
    vignetteSprite.setColor(sf::Color(255, 255, 255, 96));
    
    // Place a weapon chest if needed
    if (itemArray[level][0] != 0) {
        details.addChest(tiles, tiles.posX, tiles.posY, windowW, windowH, itemArray[level][0], pFonts);
    }
    // place life capsule chests
    if (itemArray[level][1] == 90) {
        details.addChest(tiles, tiles.posX, tiles.posY, windowW, windowH, itemArray[level][1], pFonts);
    }
}

void GameMap::update(sf::RenderWindow& window, sf::Time& elapsedTime) {
    target.clear(sf::Color::Transparent);
    // Start by getting the displacement that the player has moved, in order to update the position of all of the tiles and game objects
    xOffset = player.getWorldOffsetX();
    yOffset = player.getWorldOffsetY();
    // Draw the images
    bkg.setOffset(xOffset, yOffset);
    bkg.drawBackground(target);
    tiles.setOffset(xOffset, yOffset);
    tiles.drawTiles(target, effects.getGlowSprs(), effects.getGlowSprs2(), level);
    effects.getGlowSprs2()->clear();
    // Update the overworld objects based on the displacement of the player
    details.update(xOffset, yOffset, effects, player.getSprIndex(), tiles.walls, effects.getGlowSprs(), effects.getGlowSprs2(), UI, *pFonts, player, pInput, &ssc, elapsedTime);
    // Draw the details / add them to the game objects vector
    details.draw(gameObjects, gameShadows, target);
    // Update the enemy objects in the game based on the player's displacement
    en.updateEnemies(gameObjects, gameShadows, xOffset, yOffset, effects, tiles.walls, player.isdead(), &details, &tiles, &ssc, *pFonts, elapsedTime);
    // Draw the lower layer of the effects, that is the ones that should show up behind the player sprite
    effects.drawLower(target);
    
    if (player.visible) {
        // Draw the player to the window, as long as the object is visible
        player.draw(gameObjects, gameShadows, tiles, effects, details, sndCtrl, UI, pInput, target, *pFonts, elapsedTime);
    }
    
    // If player was hit rumble the screen.
    if (player.scrShakeState) {
        ssc.rumble();
    }
    
    effects.getGlowSprs()->clear();
    // Update the positions of all the effect objects
    effects.update(xOffset, yOffset, &ssc, elapsedTime);
    
    // Draw shadows to the window
    if (!gameShadows.empty()) {
        for (auto & element : gameShadows) {
            target.draw(std::get<0>(element));
        }
    }
    gameShadows.clear();
    
    // Sort the game object based on y-position (runtime for this is fine, only sorts objects inside the window, roughly 10 in most cases)
    std::sort(gameObjects.begin(), gameObjects.end(), sortKey());
    
    lightingMap.clear(sf::Color::Transparent);
    
    // Now draw the sorted list of game objects to the window
    if (!gameObjects.empty()) {
        for (auto & element : gameObjects) {
            // Retrieve each sprite from the tuple and draw it to the window
            if (std::get<2>(element) == 0) {
                // Darken the sprite's color channels according to the ambient conditions of the current area
                std::get<0>(element).setColor(sf::Color(objectShadeColor.r, objectShadeColor.g, objectShadeColor.b, std::get<0>(element).getColor().a));
                lightingMap.draw(std::get<0>(element));
            } else if (std::get<2>(element) == EFFECT_OBJECT) {
                // If the contents of the third element in the tuple correspond to an effect, draw the sprite without darkening it
                lightingMap.draw(std::get<0>(element));
            } else if (std::get<2>(element) == RED_TARGET) {
                // Shade red
                colorShader.setParameter("targetColor", sf::Vector3f(0.98, 0.22, 0.03));
                lightingMap.draw(std::get<0>(element), &colorShader);
            } else if (std::get<2>(element) == WHITE_TARGET) {
                // Shade white
                colorShader.setParameter("targetColor", sf::Vector3f(1.00, 1.00, 1.00));
                lightingMap.draw(std::get<0>(element), &colorShader);
            } else if (std::get<2>(element) == BLUE_TARGET) {
                // Shade blue
                colorShader.setParameter("targetColor", sf::Vector3f(0.35, 0.35, 0.69));
                lightingMap.draw(std::get<0>(element), &colorShader);
            } else if (std::get<2>(element) == CRIMSON_TARGET) {
                // Shade crimson
                colorShader.setParameter("targetColor", sf::Vector3f(0.94, 0.09, 0.34));
                lightingMap.draw(std::get<0>(element), &colorShader);
            } else if (std::get<2>(element) == ARCTIC_TARGET) {
                // Shade arctic, sort of a bright blue-greenish color
                colorShader.setParameter("targetColor", sf::Vector3f(0.29, 0.99, 0.99));
                lightingMap.draw(std::get<0>(element), &colorShader);
            }
        }
    }
    
    // Draw lights to the objects
    sf::Color blendAmount(185, 185, 185, 255);
    sf::Sprite tempSprite;
    for (auto element : *effects.getGlowSprs2()) {
        tempSprite = *element;
        tempSprite.setColor(blendAmount);
        tempSprite.setPosition(tempSprite.getPosition().x, tempSprite.getPosition().y - 12);
        lightingMap.draw(tempSprite, sf::BlendMode(sf::BlendMode(sf::BlendMode::SrcAlpha, sf::BlendMode::One, sf::BlendMode::Add, sf::BlendMode::DstAlpha, sf::BlendMode::Zero, sf::BlendMode::Add)));
    }
    
    // Display the lighting map
    lightingMap.display();
    sf::Sprite sprite(lightingMap.getTexture());
    target.draw(sprite);
    
    // Now clear out the vectors for the next round of drawing
    gameObjects.clear();
    
    effects.draw(target, gameObjects);
    
    bkg.drawForeground(target);
    
    // Display the render texture target
    target.display();
    
    // Finally draw it to the window
    if (UI.isVisible()) {
        if (computeBlur) {
            // Blurring takes a lot of cpu resources. Limit the framerate
            finalPass.clear(sf::Color::Transparent);
            sf::Vector2u textureSize = target.getSize();
            // Get the blur amount from the UI controller
            float blurAmount = UI.getBlurAmount();
            blurShader.setParameter("blur_radius", sf::Vector2f(0.f, blurAmount / textureSize.y));
            finalPass.draw(sf::Sprite(target.getTexture()), &blurShader);
            finalPass.display();
            blurShader.setParameter("blur_radius", sf::Vector2f(blurAmount / textureSize.x, 0.f));
            finalSprite.setTexture(finalPass.getTexture());
            window.draw(finalSprite, &blurShader);
        } else {
            // If the UI interface is not opening or closing, reuse some of the previously created blur resources
            // When the menu is opened, the game should be paused anyway, so it's fine to use a static image
            window.draw(finalSprite, &blurShader);
        }
    } else {
        window.draw(sf::Sprite(target.getTexture()));
    }
    
    // Draw a nice vignette effect over the entire window, but behind the UI overlay
    window.draw(vignetteSprite, sf::BlendMultiply);

    if (!player.isdead()) {
        UI.dispDeathSeq();
        pFonts->terminateCaptions();
        // If the death sequence is complete and the UI controller is finished playing its animation
        if (UI.isComplete() && pInput->zPressed()) {
            // Reset the UI controller
            UI.reset();
            // Reset the player
            player.reset();
            // Reset the map
            level = -1;
            initLoot(itemArray);
            enemySelectVec.clear();
            Reset();
            // Set the max health back to 3
            pFonts->updateMaxHealth(4);
            pFonts->setWaypointText(level);
            dispEntryBeam = false;
        }
        computeBlur = UI.drawMenu(window, &player, details.getUIStates(), *pFonts, effects, xOffset, yOffset, pInput, elapsedTime);
    }
    else {
        if (level != 0) {
            computeBlur = UI.drawMenu(window, &player, details.getUIStates(), *pFonts, effects, xOffset, yOffset, pInput, elapsedTime);
            // Pass the player's health to the font controller
            pFonts->updateHealth(player.getHealth());
            // Draw all of the game text to the window
            pFonts->print(window);
        }
    }
    // Update the screen shake controller
    ssc.update(player);
    
    window.setView(worldView);
    
    if (dispEntryBeam) {
        // Cast the beam's glow to the overworld
        effects.getGlowSprs()->push_back(&beamGlowSpr);
        effects.getGlowSprs2()->push_back(&beamGlowSpr);

        // Get the current size of the beam shape
        sf::Vector2f v2 = entryBeam.getSize();
        // Keep track of the original size of the beam, makes it easier later to update the shape position
        double originalYval = v2.y;
        // Until the beam reaches the player position at the center of the window, increase it's height and move it down
        if (v2.y < 518) {
            v2.y *= 1.11;
            sf::Color c = beamGlowSpr.getColor();
            c.r += 5;
            c.b += 5;
            c.g += 5;
            beamGlowSpr.setColor(c);
        } else if (v2.y > 518) {
            player.visible = true;
            // Call a function within the screen shake controller to shake the screen
            ssc.shake();
            v2.y = 518;
            // Add a warp smoke effect when the player appears, purely aesthetic
            effects.addWarpEffect(windowW / 2 - 8, windowH / 2 + 16);
            // Add a warp impack detail to the overworld where the player landed
            details.addWarpImpact(windowW / 2 - 12, windowH / 2 + 18);
        } else {
            player.deActivate();
        }
        // Get the current color of the beam shape, in order to decrement its alpha value
        sf::Color c1 = entryBeam.getFillColor();
        // Decrement the alpha value of the beam shape
        if (v2.y == 518) {
            c1.a *= 0.88;
            sf::Color c = beamGlowSpr.getColor();
            c.r -= 8;
            c.b -= 8;
            c.g -= 8;
            beamGlowSpr.setColor(c);
        }
        // Update the shape color
        entryBeam.setFillColor(c1);
        // Update the size of the beam shape
        entryBeam.setSize(v2);
        // Reset the shape's position based on the distance that the player has moved and the beam's previous position
        entryBeam.setPosition(entryBeam.getPosition().x, entryBeam.getPosition().y + 0.5 * (originalYval - v2.y));
        // Draw the shape to the window
        window.draw(entryBeam);
        
        if (c1.a < 6) {
            // Beam has faded out, so now stop drawing it
            dispEntryBeam = false;
            beamGlowSpr.setColor(sf::Color(0, 0, 0, 255));
            // Reset the initial values for the shape so that the program can draw it again next level
            sf::Vector2f v2(4, 8);
            entryBeam.setPosition(windowW/2 - 2, -68);
            entryBeam.setSize(v2);
            entryBeam.setFillColor(sf::Color(104, 255, 229, 180));
            // Reactivate the player now that the beam animation has finished
            player.activate();
        }
    }
    
    if (transitionIn) {
        sf::Color c = transitionShape.getFillColor();
        if (c.a > 10) {
            c.a -= 10;
        }
        transitionShape.setFillColor(c);
        window.draw(transitionShape);

        if (--transitionDelay == 0) {
            transitionIn = false;
            dispEntryBeam = true;
            // Allow the teleporter animation to play again
            animationBegin = false;
            transitionDelay = 65;
        }
    }
    
    // Check if the player is close to a teleporter. If so, go to the next level
    if ((std::abs(player.getPosX() - details.getTeleporter()->getxPos()) < 10 && std::abs(player.getPosY() - details.getTeleporter()->getyPos() + 12) < 8) /*&& player.isActive()*/) {
        // Center the player over the teleporter for the duration of the teleport animation (ie center the world under the player)
        if (!animationBegin) {
            player.setWorldOffsetX(xOffset + (player.getPosX() - details.getTeleporter()->getxPos()) + 2);
            player.setWorldOffsetY(yOffset + (player.getPosY() - details.getTeleporter()->getyPos()) + 16);
            beamExpanding = true;
            animationBegin = true;
            // Force all captions closed
            pFonts->terminateCaptions();
        }
        player.deActivate();
        // Cast the beam's glow to the overworld
        if (!transitioning) {
            window.draw(teleporterBeam);
            effects.getGlowSprs()->push_back(&beamGlowSpr);
            effects.getGlowSprs2()->push_back(&beamGlowSpr);
        }
        sf::Color beamColor = teleporterBeam.getFillColor();
        sf::Vector2f v1 = teleporterBeam.getSize();
        if (beamExpanding) {
            if (beamColor.a < 246) {
                beamColor.a += 5;
            }
            sf::Color c = beamGlowSpr.getColor();
            c.r += 4;
            c.b += 4;
            c.g += 4;
            beamGlowSpr.setColor(c);
            teleporterBeam.setFillColor(beamColor);
            if (v1.y < 300) {
                v1.y += 8;
                teleporterBeam.setPosition(teleporterBeam.getPosition().x, teleporterBeam.getPosition().y - 8);
            }
            
            if (v1.y > 260 && v1.x < 22) {
                double temp = v1.x;
                v1.x *= 1.18;
                teleporterBeam.setPosition(teleporterBeam.getPosition().x - (v1.x - temp) * 0.5, teleporterBeam.getPosition().y);
            } else if (v1.x > 21) {
                beamExpanding = false;
                player.visible = false;
            }
            teleporterBeam.setSize(v1);
        } else if (!beamExpanding) {
            double temp = v1.x;
            v1.x *= 0.92;
            beamColor.a -= 1.4;
            sf::Color c = beamGlowSpr.getColor();
            if (c.r > 6) {
                c.r -= 6;
                c.b -= 6;
                c.g -= 6;
            }
            beamGlowSpr.setColor(c);
            teleporterBeam.setFillColor(beamColor);
            teleporterBeam.setPosition(teleporterBeam.getPosition().x - (v1.x - temp) * 0.5, teleporterBeam.getPosition().y);
            teleporterBeam.setSize(v1);
            if (v1.x < 0.5) {
                transitioning = true;
                //teleporterCond = true;
                beamGlowSpr.setColor(sf::Color(0, 0, 0, 255));
                
            }
        }
        
        // Draw captions to the window
        if (!UI.isVisible()) {
            pFonts->update(window, xOffset, yOffset);
            window.setView(worldView);
        }
        
        if (transitioning) {
            if (level == 0) {
                if (transitionDelay < 130) {
                    sf::Color c = pFonts->getTitle()->getColor();
                    if (c.a > 4) {
                        c.a -= 4;
                    } else {
                        c.a = 0;
                    }
                    if (transitionDelay < 90) {
                        sf::Color c2 = transitionShape.getFillColor();
                        if (c2.a < 253) {
                            c2.a += 3;
                        }
                        transitionShape.setFillColor(c2);
                        window.draw(transitionShape);
                    }
                    pFonts->drawTitle(c.a, window);
                } else {
                    pFonts->drawTitle(255, window);
                }
                
                if (--transitionDelay == 0) {
                    transitioning = false;
                    transitionDelay = 55;
                    teleporterCond = true;
                    titleSpr.setColor(sf::Color(255, 255, 255, 255));
                }
            } else {
                if (transitionDelay < 50) {
                    sf::Color c = transitionShape.getFillColor();
                    if (c.a < 250) {
                        c.a += 5;
                    }
                    transitionShape.setFillColor(c);
                    window.draw(transitionShape);
                }
                if (--transitionDelay == 0) {
                    transitioning = false;
                    transitionDelay = 30;
                    teleporterCond = true;
                    //transitionShape.setFillColor(sf::Color(255, 255, 255, 255));
                }
            }
        }
    }
}

void GameMap::Reset() {
    //Increment the current level
    level += 1;
    pFonts->setWaypointText(level);
    //Clear all the vectors before re-initializing them, we don't want objects from the previous map showing up again!
    tiles.clear();
    effects.clear();
    details.clear();
    player.setWorldOffsetX(0);
    player.setWorldOffsetY(0);
    en.clear();
    teleporterCond = 0;
    int count;
    
    if (level == 0) {
        objectShadeColor = sf::Color(190, 190, 210, 255);
    } else if (level > 0 && level <= BOSS_LEVEL_1) {
        objectShadeColor = sf::Color(190, 190, 210, 255);
        pFonts->setZoneText(0);
    } else if (level > BOSS_LEVEL_1) {
        objectShadeColor = sf::Color(210, 195, 195, 255);
        pFonts->setZoneText(1);
    }
    
    vignetteSprite.setColor(sf::Color(255, 255, 255, 255));

    // If not on a boss level...
    if (level != BOSS_LEVEL_1) {
        //Now call the mapping function again to generate a new map, and make sure it's large enough
        count = mappingFunction(tiles.mapArray, level, level < BOSS_LEVEL_1);
        while (count < 150) {
            count = mappingFunction(tiles.mapArray, level, level < BOSS_LEVEL_1);
        }
    } else if (level == 0) {
    
    } else {
        mappingFunction(tiles.mapArray, level, level < BOSS_LEVEL_1);
    }
    
    //Now lets rebuild the map from the new array, using the same function from the tileController class constructor
    tiles.rebuild(itemArray, level);
    // Now let the background handler know what tileset the tilecontroller is using
    bkg.setBkg(tiles.getWorkingSet());
    /*Of course, the tile controller needs to know how big the window is so that it can find the center when drawing the tiles (tiles outside the window don't draw, and the player object is in the center of the window--we don't want to draw it inside a wall!)
    */
    tiles.setPosition((windowW / 2) - 16, (windowH / 2));
    bkg.setPosition((tiles.posX / 2) + 206, tiles.posY / 2);
    
    // Perhaps there's a better way to not check the same condition multiple times, without tons of copy-pasting
    if (level != BOSS_LEVEL_1 && level != 0) {
        details.addTeleporter(tiles, tiles.posX, tiles.posY, windowW, windowH, pFonts);
        
        // Now initialize enemies for the map based on level, and store sum of their exp values in a variable
        count = initEnemies(this);
        
        // Add broken down robots to the map (if correct tileset for it
        if (level < 10) {
            details.addDamagedRobots(tiles, tiles.posX, tiles.posY, pFonts);
        }
        // Tell the UI controller what that variable is
        UI.setEnemyValueCount(count);
        
        // Place a weapon chest is needed
        if (itemArray[level][0] != 0) {
            details.addChest(tiles, tiles.posX, tiles.posY, windowW, windowH, itemArray[level][0], pFonts);
        }
        // place life capsule chests
        if (itemArray[level][1] == 90) {
            details.addChest(tiles, tiles.posX, tiles.posY, windowW, windowH, itemArray[level][1], pFonts);
        }
        
        effects.getGlowSprs()->clear();
        effects.getGlowSprs2()->clear();
        
        size_t len;
        Teleporter* pTeleporter = details.getTeleporter();
        if (level < BOSS_LEVEL_1) {
            // Put rock/pillar detail things on map
            getRockPositions(tiles.mapArray, rockPositions);
            for (auto element : rockPositions) {
                details.addRock(tiles, tiles.posX, tiles.posY - 35, element.x, element.y, pFonts);
            }
            rockPositions.clear();
            // Delete rocks close to the teleporter
            std::vector<Rock>* pRocks = details.getRocks();
            for (auto it = pRocks->begin(); it != pRocks->end();) {
                if (fabsf(it->getxPos() - pTeleporter->getxPos()) < 80 && fabsf(it->getyPos() - pTeleporter->getyPos()) < 80) {
                    it = pRocks->erase(it);
                } else {
                    ++it;
                }
            }
        }
        
        // Put light sources on the map
        getLightingPositions(tiles.mapArray, lightPositions);
        len = lightPositions.size();
        for (auto i = 0; i < len; i++) {
            details.addLamplight(tiles, tiles.posX - 2, tiles.posY - 16, lightPositions[i].x, lightPositions[i].y, windowW, windowH);
        }
        lightPositions.clear();
        
        // Delete lamps near the teleporter (light blending is additive, it would be too bright if they were close together)
        std::vector<LampLight>* pLamps = details.getLamps();
        for (auto i = 0; i < details.getLamps()->size(); i++) {
            if (fabsf((*pLamps)[i].getxPos() - pTeleporter->xPos) < 90 && fabsf((*pLamps)[i].getyPos() - pTeleporter->yPos) < 90) {
                (*pLamps)[i] = (*pLamps).back();
                (*pLamps).pop_back();
            }
        }
    }
    
    else if (level == BOSS_LEVEL_1) {
        tiles.teleporterLocation.x = 34;
        tiles.teleporterLocation.y = 17;
        details.addTeleporter(tiles, tiles.posX, tiles.posY, windowW, windowH, pFonts);
        tiles.teleporterLocation.x = 38;
        details.addTeleporter(tiles, tiles.posX, tiles.posY, windowW, windowH, pFonts);
        details.addLamplight(tiles, tiles.posX - 2, tiles.posY - 16, 39, 31, windowW, windowH);
        details.addLamplight(tiles, tiles.posX - 2, tiles.posY - 16, 33, 26, windowW, windowH);
    } else if (level == 0) {
        details.addLamplight(tiles, tiles.posX - 180, tiles.posY + 200, 5, 6, windowW, windowH);
        details.addLamplight(tiles, tiles.posX - 180, tiles.posY + 200, 5, 0, windowW, windowH);
        details.addLamplight(tiles, tiles.posX - 170, tiles.posY + 200, 11, 11, windowW, windowH);
        details.addLamplight(tiles, tiles.posX - 180, tiles.posY + 200, 10, -9, windowW, windowH);
        details.addDoor(tiles.posX - 192, tiles.posY + 301, 6, 0, windowW, windowH);
        details.addPod(tiles.posX, tiles.posY + 33, 3, 17, pFonts);
        tiles.teleporterLocation.x = 8;
        tiles.teleporterLocation.y = -7;
        details.addTeleporter(tiles, tiles.posX - 178, tiles.posY + 284, windowW, windowH, pFonts);
        for (auto it = global_levelZeroWalls.begin(); it != global_levelZeroWalls.end(); ++it) {
            wall w;
            w.setXinit(it->first);
            w.setYinit(it->second);
            tiles.walls.push_back(w);
        }
    }
    
    // Reset the teleporter beam coordinates, alpha, & size
    sf::Vector2f v1(2, 1);
    teleporterBeam.setPosition(windowW/2 - 1, windowH/2 + 36);
    teleporterBeam.setSize(v1);
    teleporterBeam.setFillColor(sf::Color(104, 255, 229, 6));
    if (level != 0)
        transitionIn = true;
}

Player GameMap::getPlayer() {
    return player;
}

detailController GameMap::getDetails() {
    return details;
}

bool GameMap::getTeleporterCond() {
    return teleporterCond;
}

std::vector<std::pair<int, int>>* GameMap::getEnemySelectVec() {
    return &enemySelectVec;
}

int* GameMap::getIdealLvs() {
    return idealLvs;
}

int GameMap::getLevel() {
    return level;
}
