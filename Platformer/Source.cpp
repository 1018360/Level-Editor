#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING //VS2109
#include "Editor.h"
#include "MenuHeader.h"
#include "WindowHeader.h"
#include "Game.h"
#include "Audio.h"
#include <experimental/filesystem>


namespace fs = std::experimental::filesystem;


std::vector<std::string> saves;
int curLevel = 0;
void GetAllSaveFiles()
{
	std::string path = "\.";
	std::string ext = ".sav";
	saves.clear();
	for (auto& p : fs::recursive_directory_iterator(path))
	{
		if (p.path().extension() == ext)
		{
			std::cout << p.path().filename().generic_string() << '\n';
			std::string file = p.path().filename().generic_string();
			saves.push_back(file);
		}
	}
}
int main()
{
	EditorClass myEditor;
	MainMenu myMainMenu;
	GameClass myGame;
	MainRenderWindow mainWindow;
	
	
	mainWindow.window.create(sf::VideoMode(mainWindow.windowWidth, mainWindow.windowHeight), "My Program", sf::Style::Titlebar | sf::Style::Close);

	while (mainWindow.window.isOpen())
	{
		sf::Event event;
		while (mainWindow.window.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				mainWindow.close();
				break;
			}
		}

		switch (myMainMenu.myMode.currentMode)
		{
		case mode::MainMenu:
			if (!myMainMenu.Start())
			{
				return EXIT_FAILURE;
			}
			myMainMenu.Update(mainWindow);
			break;
		case mode::Editor:
			if (!myEditor.editorActive)
			{
				if (!myEditor.Start(mainWindow))
				{
					return EXIT_FAILURE;
				}
			}
			myEditor.Update(mainWindow);
			break;
		case mode::Game:
			if (!myGame.gameActive)
			{
				if (!myGame.Start(mainWindow))
				{
					return EXIT_FAILURE;
				}
			}
			myGame.Update(mainWindow);
			break;
		default:
			break;
		}
	}
	
	return 0;
}

bool MainMenu::Start()
{
	return true;
}

void MainMenu::Update(MainRenderWindow& mainWindow)
{
	while (menuActive)
	{
		mainWindow.worldPos = mainWindow.window.mapPixelToCoords(sf::Mouse::getPosition(mainWindow.window));
		levelEditorButton.checkClick(std::bind(&MainMenu::ChangeMode, this, mode::Editor), mainWindow.worldPos);
		gameButton.checkClick(std::bind(&MainMenu::ChangeMode, this, mode::Game), mainWindow.worldPos);
		mainWindow.clear();
		mainWindow.window.draw(levelEditorButton);
		mainWindow.window.draw(gameButton);
		mainWindow.window.display();
	}
}

EditorClass::EditorClass()
{
	for (int i = 0; i < x; i++)
	{
		tile[i] = new Tile[y];
	}
}


bool EditorClass::Start(MainRenderWindow &mainWindow)
{
	//setup our views
	toolsView = sf::View(sf::FloatRect(0, 0, mainWindow.windowWidth * 0.045f, mainWindow.windowHeight));
	toolsView.setViewport(sf::FloatRect(0, 0, 0.045f, 1));
	levelEditView = sf::View(sf::FloatRect(0, 0, mainWindow.windowWidth, mainWindow.windowHeight));
	levelEditView.setViewport(sf::FloatRect(0.03f, 0, 1, 1));

	//setup variables to paint with
	curTileType = Tile::Type::Platform;
	curActorType = Actor::Type::Coin;

	for (int i = 0; i < 9; i++)
	{
		tileButton[i].init(10, (32 + 5) * i + 150);
	}
	//setting each button to have a tile/actor type
	tileButton[0].ChangeTile(Tile::Type::Sky);
	tileButton[1].ChangeTile(Tile::Type::Platform);
	tileButton[2].ChangeTile(Tile::Type::Lava);
	tileButton[3].ChangeActor(Actor::Type::None);
	tileButton[4].ChangeActor(Actor::Type::Player);
	tileButton[5].ChangeActor(Actor::Type::Enemy);
	tileButton[6].ChangeActor(Actor::Type::Coin);
	tileButton[7].ChangeActor(Actor::Type::Spike);
	tileButton[8].ChangeActor(Actor::Type::Exit);

	for (int i = 0; i < x; i++)
	{
		for (int j = 0; j < y; j++)
		{
			tile[i][j].init(i * 32 + ((mainWindow.windowWidth / 2) - ((32 * x) / 2)), j * 32);
		}
	}
	editorActive = true;
	return true;
}

void EditorClass::Update(MainRenderWindow &mainWindow)
{
	
	//prep window for displaying stuff
	mainWindow.window.clear(sf::Color::White);
	mainWindow.window.setView(toolsView);
	//track mouse pos
	worldPos = mainWindow.window.mapPixelToCoords(sf::Mouse::getPosition(mainWindow.window));

	//bind our save/load buttons to save/load functions
	tools.saveButton.checkClick(std::bind(&EditorClass::save, this, playerInput, tile), worldPos);
	tools.loadButton.checkClick(std::bind(&EditorClass::load, this, playerInput, tile), worldPos);

	//loop through our toolbar to check for clicks
	for (int i = 0; i < 9; i++)
	{
		if (tileButton[i].mouseOver(worldPos))
		{
			tileButton[i].sprite.setScale(sf::Vector2f(1.1f, 1.1f));
			tileButton[i].actor.sprite.setScale(sf::Vector2f(1.1f, 1.1f));
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				if (i < 3)
				{
					actorNotTile = false;
					curTileType = tileButton[i].type;
				}
				else //3 and above are actors
				{
					actorNotTile = true;
					curActorType = tileButton[i].actor.type;
				}
			}
		}
		else
		{
			tileButton[i].sprite.setScale(sf::Vector2f(1.f, 1.f));
			tileButton[i].actor.sprite.setScale(sf::Vector2f(1.f, 1.f));
		}
		mainWindow.window.draw(tileButton[i]);
	}

	mainWindow.window.draw(tools);
	mainWindow.window.setView(levelEditView);
	worldPos = mainWindow.window.mapPixelToCoords(sf::Mouse::getPosition(mainWindow.window), mainWindow.window.getView());

	//draw our tiles
	for (int i = 0; i < x; i++)
	{
		for (int j = 0; j < y; j++)
		{
			//check for click
			if (tile[i][j].mouseOver(worldPos) && sf::Mouse::isButtonPressed(sf::Mouse::Left))
			{
				if (!actorNotTile)
				{
					tile[i][j].ChangeTile(curTileType);
				}
				else
				{
					tile[i][j].ChangeActor(curActorType);
				}
			}
			tile[i][j].RefreshTile();
			mainWindow.window.draw(tile[i][j]);
		}
	}

	sf::Event event;
	while (mainWindow.window.pollEvent(event))
	{
		if (event.type == sf::Event::TextEntered)
		{
			if (event.text.unicode == 8 && playerInput.size() != 0)
			{
				playerInput.pop_back();
				std::cout << playerInput << std::endl;
			}
			else if (event.text.unicode < 128)
			{
				playerInput += static_cast<char>(event.text.unicode);
				std::cout << playerInput << std::endl;
			}
			inputField.text.setString(playerInput);
		}   
	}
	inputField.UpdatePos(sf::Vector2f(2, 52));
	mainWindow.window.draw(inputField);
	mainWindow.window.display();
}

bool GameClass::Start(MainRenderWindow& mainWindow)
{
	GetAllSaveFiles();
	//setup of game, init tiles
	for (int i = 0; i < x; i++)
	{
		for (int j = 0; j < y; j++)
		{
			tile[i][j].init(i * 32 + ((mainWindow.windowWidth / 2) - ((32 * x) / 2)), j * 32);
		}
	}
	//load save file
	if (saves.size() != 0)
	{
		LoadLevel(saves[curLevel], tile);
	}
	else
	{
		std::cout << "Error, no saved levels!" << std::endl;
	}

	player.nextPos = player.getPosition();
	gameActive = true;
	audioClass.playAudio();
	return true;
}

void GameClass::Update(MainRenderWindow& mainWindow)
{
	mainWindow.window.clear(sf::Color::White);
	deltaTime = clock.restart().asSeconds();
	//Controls!
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D))
	{
		
		//move right
		if (player.isGrounded)
		{
			player.velocity.x += player.speed * deltaTime;
			player.velocity.y += deltaTime;
		}
		else
		{
			player.velocity.x += player.speed / 3 * deltaTime;
		}
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A))
	{
		//move left
		if (player.isGrounded)
		{
			player.velocity.x -= player.speed * deltaTime;
			player.velocity.y += deltaTime;
		}
		else
		{
			player.velocity.x -= player.speed / 3 * deltaTime;
		}
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
	{
		//jump
		if (player.isGrounded)
		{
			player.isGrounded = false;
			player.velocity.y += -player.jumpSpeed + deltaTime;
			audioClass.sound.setBuffer(audioClass.jumpSB);
			audioClass.sound.play();
		}
		
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q))
	{
		mainWindow.close();
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::R))
	{
		LoadLevel("Level1.sav", tile);
	}

	//friction
	if (player.isGrounded)
	{
		if (!player.isAnyKeyPressed())
		{
			player.velocity.x -= friction * deltaTime * (sign(player.velocity.x));
		}
	}

	//maximum abs velcoity
	if (abs(player.velocity.x) > 0.6f)
	{
		player.velocity.x = 0.6f * sign(player.velocity.x);
	}
	//max vertical velocity
	if (player.velocity.y < 1.0f)
	{
		player.velocity.y += gravity * deltaTime;
	}
	else if (player.velocity.y < -1.0f)
	{
		player.velocity.y = -1.0f;
	}

	//this is the players next potential position, if they arent obstructed
	player.nextPos = player.getPosition() + player.velocity;
	//project a hitbox
	player.nextRect = sf::FloatRect(player.nextPos, sf::Vector2f(32.f, 32.f));
	player.isGrounded = false;
	//loop for collision with tiles as well as drawing them
	for (int i = 0; i < x; i++)
	{
		for (int j = 0; j < y; j++)
		{
			//draw tiles
			tile[i][j].RefreshTile();
			mainWindow.window.draw(tile[i][j]);

			//check for collisions
			if (tile[i][j].type == Tile::Type::Platform)
			{
				Collision pcol = player.CollisionCheck(tile[i][j].sprite.getGlobalBounds());
				if (pcol.hit)
				{
					//hit something vertically
					if (pcol.dir.x == 0)
					{
						//hitting the tile below us
						if (pcol.dir.y >= 0.0f)
						{
							//we're on top of the tile
							player.nextPos.y = tile[i][j].sprite.getGlobalBounds().top - 32 - 0.1f;
							player.isGrounded = true;
						}
						else
						{
							//instead of using 32 we should be getting the height of the sprite, incase sizes change
							player.nextPos.y = tile[i][j].sprite.getGlobalBounds().top + 32 + 0.1f;
							player.velocity.y = 0.0f;
						}
					}
					else //horizontal hit
					{
						//right side of the tile?
						if (pcol.dir.x >= 0.0f)
						{
							player.nextPos.x = tile[i][j].sprite.getGlobalBounds().left - 32;
							player.velocity.x = 0.0f;
						}
						else
						{
							player.nextPos.x = tile[i][j].sprite.getGlobalBounds().left + 32;
							player.velocity.x = 0.0f;
						}
					}
				}
			}
			else if (tile[i][j].type == Tile::Type::Lava)
			{
				Collision pcol = player.CollisionCheck(tile[i][j].sprite.getGlobalBounds());
				if (pcol.hit)
				{
					//remove a life
					player.lives--;
					//reset position
					player.Respawn();
					std::cout << "Player hit lava" << std::endl;
					audioClass.sound.setBuffer(audioClass.deathSB);
					audioClass.sound.play();
					if (player.lives == 0)
					{
						//game over(todo - add gameover state/screen)
						std::cout << "Game Over!" << std::endl;
						LoadLevel("ZZZYou Lose.sav", tile);
						//mainWindow.close();
					}
				}
			}
			else if (tile[i][j].actor.type == Actor::Type::Coin)
			{
				//add coin, then destroy the coin (changing type to none)
				Collision pcol = player.CollisionCheck(tile[i][j].actor.sprite.getGlobalBounds());
				if (pcol.hit)
				{
					std::cout << "Player Grabbed Coin!" << std::endl;
					player.coins++;
					tile[i][j].ChangeActor(Actor::Type::None);
					audioClass.sound.setBuffer(audioClass.coinSB);
					audioClass.sound.play();
				}
			}
			else if (tile[i][j].actor.type == Actor::Type::Spike)
			{
				Collision pcol = player.CollisionCheck(tile[i][j].actor.sprite.getGlobalBounds());
				if (pcol.hit)
				{
					//remove life
					player.lives--;
					//reset position
					player.Respawn();
					std::cout << "Player hit spike" << std::endl;
					audioClass.sound.setBuffer(audioClass.deathSB);
					audioClass.sound.play();
					if (player.lives == 0)
					{
						//transition to our game over screen
						std::cout << "Game Over!" << std::endl;
						LoadLevel("ZZZYou Lose.sav", tile);
						//mainWindow.close();
					}
				}
			}
			else if (tile[i][j].actor.type == Actor::Type::Enemy)
			{
				Collision pcol = player.CollisionCheck(tile[i][j].actor.sprite.getGlobalBounds());
				if (pcol.hit)
				{
					//hit something vertically
					if (pcol.dir.x == 0)
					{
						//is the player hitting from above?
						if (pcol.dir.y >= 0.0f)
						{
							//we're on top of the enemy
							//kill enemy(this is temport code until we give a proper death state thing)
							tile[i][j].ChangeActor(Actor::Type::None);
						}
						else
						{
							//enemy above
							player.lives--;
							player.Respawn();
							audioClass.sound.setBuffer(audioClass.deathSB);
							audioClass.sound.play();
							if (player.lives == 0)
							{
								//insert game over transition
								std::cout << "Game Over!" << std::endl;
								LoadLevel("ZZZYou Lose.sav", tile);
								//mainWindow.close();
							}
						}
					}
					else //horizontal hit, instant death
					{
						player.lives--;
						player.Respawn();
						audioClass.sound.setBuffer(audioClass.deathSB);
						audioClass.sound.play();
						if (player.lives == 0)
						{
							//insert game over transition
							std::cout << "Game Over!" << std::endl;
							LoadLevel("ZZZYou Lose.sav", tile);
							//mainWindow.close();
						}
					}
				}
			}
			else if (tile[i][j].actor.type == Actor::Type::Exit)
			{
				Collision pcol = player.CollisionCheck(tile[i][j].actor.sprite.getGlobalBounds());
				if (pcol.hit)
				{
					if (curLevel != saves.size() - 1)
					{
						curLevel++;
						LoadLevel(saves[curLevel], tile);
					}
					else
					{
						//victory screen! todo make a you win! screen
						mainWindow.close();
					}
				}
			}
		}
	}
	
	//set player position
	player.setPosition(player.nextPos);
	//draw
	mainWindow.window.draw(player);
	mainWindow.window.display();
}