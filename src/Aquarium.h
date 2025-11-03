#define NOMINMAX // To avoid min/max macro conflict on Windows

#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>
#include "Core.h"


enum class AquariumCreatureType {
    NPCreature,
    BiggerFish,
    PufferFish,
    Angelfish,
    Surgeonfish,
};

enum class PowerUpType { SpeedBoost };

struct PowerUpItem {
    float x = 0.f, y = 0.f;
    float radius = 20.f;
    PowerUpType type = PowerUpType::SpeedBoost;
    std::shared_ptr<GameSprite> sprite;
};

string AquariumCreatureTypeToString(AquariumCreatureType t);

class AquariumLevelPopulationNode{
    public:
        AquariumLevelPopulationNode() = default;
        AquariumLevelPopulationNode(AquariumCreatureType creature_type, int population) {
            this->creatureType = creature_type;
            this->population = population;
            this->currentPopulation = 0;
        };
        AquariumCreatureType creatureType;
        int population;
        int currentPopulation;
};

class AquariumLevel : public GameLevel {
    public:
        AquariumLevel(int levelNumber, int targetScore)
        : GameLevel(levelNumber), m_level_score(0), m_targetScore(targetScore){};
        void ConsumePopulation(AquariumCreatureType creature, int power);
        bool isCompleted() override;
        void populationReset();
        void levelReset(){m_level_score=0;this->populationReset();}
        virtual std::vector<AquariumCreatureType> Repopulate();
    protected:
        std::vector<std::shared_ptr<AquariumLevelPopulationNode>> m_levelPopulation;
        int m_level_score;
        int m_targetScore;

};


class PlayerCreature : public Creature {
public:

    PlayerCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    void move();
    void draw() const;
    void update();
    void changeSpeed(int speed);
    void setLives(int lives) { m_lives = lives; }
    void setDirection(float dx, float dy);
    float isXDirectionActive() { return m_dx != 0; }
    float isYDirectionActive() {return m_dy != 0; }
    float getDx() { return m_dx; }
    float getDy() { return m_dy; }

    int getScore()const { return m_score; }
    int getLives() const { return m_lives; }
    int getPower() const { return m_power; }
    
    void addToScore(int amount, int weight=1) { m_score += amount * weight; }
    void loseLife(int debounce);
    void increasePower(int value) { m_power += value; }
    void reduceDamageDebounce();

    void activateSpeedBoost(float multiplier, int frames);
    bool hasSpeedBoost() const { return m_speedBoostFrames > 0; }
    int  speedBoostFramesLeft() const { return m_speedBoostFrames; }

private:
    int m_score = 0;
    int m_lives = 3;
    int m_power = 1; // mark current power lvl
    int m_damage_debounce = 0; // frames to wait after eating

    int m_baseSpeed = 0;
    int m_speedBoostFrames = 0;
    int m_speedCap = 0;
    
};

class NPCreature : public Creature {
public:
    NPCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    AquariumCreatureType GetType() {return this->m_creatureType;}
    void move() override;
    void draw() const override;
protected:
    AquariumCreatureType m_creatureType;

};

class BiggerFish : public NPCreature {
public:
    BiggerFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    void move() override;
    void draw() const override;
};

//####################### New Fishes ####################################################
class PufferFish : public NPCreature {
public:
    PufferFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    void move() override;
    void draw() const override;
private:
    int   m_tick;
    int   m_cycleLen;
    int   m_inflateLen;
    float m_baseRadius;
    float m_inflatedRadius;
};

class Angelfish : public NPCreature {
public:
    Angelfish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    void move() override;
    void draw() const override;
private:
    float m_phase;
};

class Surgeonfish : public NPCreature {
public:
    Surgeonfish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite);
    void move() override;
    void draw() const override;
private:
    int   m_tick;
    float m_targetX, m_targetY;
    static float clampf(float v, float lo, float hi) { return std::max(lo, std::min(v, hi)); }
};



class AquariumSpriteManager {
    public:
        AquariumSpriteManager();
        ~AquariumSpriteManager() = default;
        std::shared_ptr<GameSprite>GetSprite(AquariumCreatureType t);
        std::shared_ptr<GameSprite> GetPowerUpSprite(PowerUpType t) { return m_speed_powerup; }
    private:
        std::shared_ptr<GameSprite> m_npc_fish;
        std::shared_ptr<GameSprite> m_big_fish;
        std::shared_ptr<GameSprite> m_puffer_fish; //new fish
        std::shared_ptr<GameSprite> m_angelfish; //new fish
        std::shared_ptr<GameSprite> m_surgeonfish; //new fish
        std::shared_ptr<GameSprite> m_speed_powerup;
};


class Aquarium{
public:
    Aquarium(int width, int height, std::shared_ptr<AquariumSpriteManager> spriteManager);
    void addCreature(std::shared_ptr<Creature> creature);
    void addAquariumLevel(std::shared_ptr<AquariumLevel> level);
    void removeCreature(std::shared_ptr<Creature> creature);
    void clearCreatures();
    void update();
    void draw() const;
    void setBounds(int w, int h) { m_width = w; m_height = h; }
    void setMaxPopulation(int n) { m_maxPopulation = n; }
    void Repopulate();
    void SpawnCreature(AquariumCreatureType type);
    
    std::shared_ptr<Creature> getCreatureAt(int index);
    int getCreatureCount() const { return m_creatures.size(); }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    int  getPowerUpCount() const { return (int)m_powerups.size(); }
    const std::vector<PowerUpItem>& getPowerUps() const { return m_powerups; }
    void removePowerUpAt(size_t idx);


private:
    int m_maxPopulation = 0;
    int m_width;
    int m_height;
    int currentLevel = 0;
    std::vector<std::shared_ptr<Creature>> m_creatures;
    std::vector<std::shared_ptr<Creature>> m_next_creatures;
    std::vector<std::shared_ptr<AquariumLevel>> m_aquariumlevels;
    std::shared_ptr<AquariumSpriteManager> m_sprite_manager;

    std::vector<PowerUpItem> m_powerups;
    int m_powerupSpawnTimer = 0;
    void maybeSpawnPowerUp();
};


std::shared_ptr<GameEvent> DetectAquariumCollisions(std::shared_ptr<Aquarium> aquarium, std::shared_ptr<PlayerCreature> player);


class AquariumGameScene : public GameScene {
    public:
        AquariumGameScene(std::shared_ptr<PlayerCreature> player, std::shared_ptr<Aquarium> aquarium, string name)
        : m_player(std::move(player)) , m_aquarium(std::move(aquarium)), m_name(name){}
        std::shared_ptr<GameEvent> GetLastEvent(){return m_lastEvent;}
        void SetLastEvent(std::shared_ptr<GameEvent> event){this->m_lastEvent = event;}
        std::shared_ptr<PlayerCreature> GetPlayer(){return this->m_player;}
        std::shared_ptr<Aquarium> GetAquarium(){return this->m_aquarium;}
        string GetName()override {return this->m_name;}
        void Update() override;
        void Draw() override;
    private:
        void paintAquariumHUD();
        std::shared_ptr<PlayerCreature> m_player;
        std::shared_ptr<Aquarium> m_aquarium;
        std::shared_ptr<GameEvent> m_lastEvent;
        string m_name;
        AwaitFrames updateControl{5};
};


class Level_0 : public AquariumLevel  {
public:
    Level_0(int levelNumber, int targetScore) : AquariumLevel(levelNumber, targetScore) {
        // Base y un par de especies nuevas
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::NPCreature, 10));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::PufferFish, 2));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::Angelfish, 3));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::Surgeonfish, 2));
    };
};

class Level_1 : public AquariumLevel  {
public:
    Level_1(int levelNumber, int targetScore) : AquariumLevel(levelNumber, targetScore) {
        // Más población y variedad
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::NPCreature, 16));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::BiggerFish, 3));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::PufferFish, 3));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::Angelfish, 4));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::Surgeonfish, 3));
    };
};

class Level_2 : public AquariumLevel  {
public:
    Level_2(int levelNumber, int targetScore) : AquariumLevel(levelNumber, targetScore) {
        // Nivel con todo
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::NPCreature, 20));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::BiggerFish, 6));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::PufferFish, 4));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::Angelfish, 5));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::Surgeonfish, 4));
    };
};

class Level_3 : public AquariumLevel  {
public:
    Level_3(int levelNumber, int targetScore)
    : AquariumLevel(levelNumber, targetScore) {
        // más variedad, introducimos Puffer y Angelfish
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::NPCreature, 18));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::BiggerFish, 6));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::PufferFish, 4));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::Angelfish, 6));
    }
};

class Level_4 : public AquariumLevel  {
public:
    Level_4(int levelNumber, int targetScore)
    : AquariumLevel(levelNumber, targetScore) {
        // metemos Surgeonfish y subimos densidad
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::NPCreature, 20));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::BiggerFish, 8));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::Angelfish, 6));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::Surgeonfish, 6));
    }
};

class Level_5 : public AquariumLevel  {
public:
    Level_5(int levelNumber, int targetScore)
    : AquariumLevel(levelNumber, targetScore) {
        // ecosistema “adulto”: muchos peces y mezcla completa
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::NPCreature, 22));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::BiggerFish, 10));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::PufferFish, 6));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::Angelfish, 8));
        m_levelPopulation.push_back(std::make_shared<AquariumLevelPopulationNode>(AquariumCreatureType::Surgeonfish, 8));
    }
};

