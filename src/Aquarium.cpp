#include "Aquarium.h"
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <ofMain.h>




string AquariumCreatureTypeToString(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:
            return "BiggerFish";
        case AquariumCreatureType::NPCreature:
            return "BaseFish";
        case AquariumCreatureType::PufferFish: //new
            return "PufferFish";
        case AquariumCreatureType::Angelfish: //new
            return "Angelfish";
        case AquariumCreatureType::Surgeonfish: //new fishes
            return "Surgeonfish";
        default:
            return "UknownFish";
    }
}

// PlayerCreature Implementation
PlayerCreature::PlayerCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, speed, 35.0f, 1, sprite) {
    m_baseSpeed = speed;
    m_speedCap  = speed * 2;
}


void PlayerCreature::setDirection(float dx, float dy) {
    m_dx = dx;
    m_dy = dy;
    normalize();
}

void PlayerCreature::move() {
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    this->bounce();
}

void PlayerCreature::reduceDamageDebounce() {
    if (m_damage_debounce > 0) {
        --m_damage_debounce;
    }
}

void PlayerCreature::activateSpeedBoost(float multiplier, int frames) {
    const int MAX_FRAMES = 10 * 60;
    const int HARD_CAP   = m_baseSpeed * 2;

    m_speedBoostFrames = std::min(frames, MAX_FRAMES);
    int target = static_cast<int>(std::round(m_baseSpeed * multiplier));
    m_speedCap = HARD_CAP;
    m_speed    = std::min(target, HARD_CAP);

    ofLogNotice() << "Speed boost active. Speed=" << m_speed
                  << "  time left=" << m_speedBoostFrames << " frames" << std::endl;
}

void PlayerCreature::update() {
    if (m_speedBoostFrames > 0) {
        --m_speedBoostFrames;
        if (m_speedBoostFrames <= 0) {
            m_speedBoostFrames = 0;
            m_speed = m_baseSpeed;
            ofLogNotice() << "Speed boost ended. Speed reset to " << m_speed << std::endl;
        }
    }
    this->reduceDamageDebounce();
    this->move();
}


void PlayerCreature::draw() const {
    
    ofLogVerbose() << "PlayerCreature at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    if (this->m_damage_debounce > 0) {
        ofSetColor(ofColor::red); // Flash red if in damage debounce
    }
    if (m_sprite) {
        m_sprite->draw(m_x, m_y);
    }
    ofSetColor(ofColor::white); // Reset color

}

void PlayerCreature::changeSpeed(int speed) {
    m_speed = speed;
}

void PlayerCreature::loseLife(int debounce) {
    if (m_damage_debounce <= 0) {
        if (m_lives > 0) this->m_lives -= 1;
        m_damage_debounce = debounce; // Set debounce frames
        ofLogNotice() << "Player lost a life! Lives remaining: " << m_lives << std::endl;
    }
    // If in debounce period, do nothing
    if (m_damage_debounce > 0) {
        ofLogVerbose() << "Player is in damage debounce period. Frames left: " << m_damage_debounce << std::endl;
    }
}

void PlayerCreature::eatFish() {
    m_hasEatenFish = true;
}

void PlayerCreature::resetBounce() {
    m_hasEatenFish = false;
}

// NPCreature Implementation
NPCreature::NPCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, speed, 30, 1, sprite) {
    m_dx = (rand() % 3 - 1); // -1, 0, or 1
    m_dy = (rand() % 3 - 1); // -1, 0, or 1
    normalize();

    m_creatureType = AquariumCreatureType::NPCreature;
}

void NPCreature::move() {
    // Simple AI movement logic (random direction)
    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;
    if(m_dx < 0 ){
        this->m_sprite->setFlipped(true);
    }else {
        this->m_sprite->setFlipped(false);
    }
    bounce();
}

void NPCreature::draw() const {
    ofLogVerbose() << "NPCreature at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    ofSetColor(ofColor::white);
    if (m_sprite) {
        m_sprite->draw(m_x, m_y);
    }
}


BiggerFish::BiggerFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, sprite) {
    m_dx = (rand() % 3 - 1);
    m_dy = (rand() % 3 - 1);
    normalize();

    setCollisionRadius(60); // Bigger fish have a larger collision radius
    m_value = 5; // Bigger fish have a higher value
    m_creatureType = AquariumCreatureType::BiggerFish;
}

void BiggerFish::move() {
    // Bigger fish might move slower or have different logic
    m_x += m_dx * (m_speed * 0.5); // Moves at half speed
    m_y += m_dy * (m_speed * 0.5);
    if(m_dx < 0 ){
        this->m_sprite->setFlipped(true);
    }else {
        this->m_sprite->setFlipped(false);
    }

    bounce();
}

void BiggerFish::draw() const {
    ofLogVerbose() << "BiggerFish at (" << m_x << ", " << m_y << ") with speed " << m_speed << std::endl;
    this->m_sprite->draw(this->m_x, this->m_y);
}
//#################### PufferFish implementation ########################################
PufferFish::PufferFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, std::max(1, speed/2), sprite)
, m_tick(0), m_cycleLen(150), m_inflateLen(45)
, m_baseRadius(38.0f), m_inflatedRadius(54.0f) {
    do { m_dx = (rand()%3)-1; m_dy = (rand()%3)-1; } while (m_dx==0 && m_dy==0);
    normalize();
    setCollisionRadius((int)m_baseRadius);
    m_value = 4;
    m_creatureType = AquariumCreatureType::PufferFish;
}
void PufferFish::move() {
    const float MAXX = ofGetWidth()  - 20.0f;
    const float MAXY = ofGetHeight() - 20.0f;

    ++m_tick;
    int t = m_tick % m_cycleLen;
    bool inflated = (t < m_inflateLen);
    float speedFactor = inflated ? 0.55f : 1.0f;

    setCollisionRadius(inflated ? (int)m_inflatedRadius : (int)m_baseRadius);

    float wobble = std::sin(0.06f * m_tick) * 0.35f;
    m_x += m_dx * (m_speed * speedFactor) + wobble;
    m_y += m_dy * (m_speed * speedFactor) - wobble * 0.6f;

    if (m_sprite) m_sprite->setFlipped(m_dx < 0);
    bounce();

    if (m_x <= 0 || m_x + getCollisionRadius()*2 >= MAXX
     || m_y <= 0 || m_y + getCollisionRadius()*2 >= MAXY) {
        do { m_dx = (rand()%3)-1; m_dy = (rand()%3)-1; } while (m_dx==0 && m_dy==0);
        normalize();
    }
}





void PufferFish::draw() const {
    if (m_sprite) m_sprite->draw(m_x, m_y);
}
//############################ AngelFish Implementation #####################################
Angelfish::Angelfish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, std::max(1, speed-1), sprite), m_phase(0.0f) {
    m_dx = (rand()%2==0) ? 0.5f : -0.5f;
    m_dy = 1.0f;
    normalize();
    setCollisionRadius(44);
    m_value = 3;
    m_creatureType = AquariumCreatureType::Angelfish;
}

void Angelfish::move() {
    const float MAXX = ofGetWidth()  - 20.0f;
    const float MAXY = ofGetHeight() - 20.0f;

    m_phase += 0.05f;
    float vy = 1.2f + std::sin(m_phase) * 0.6f;

    m_x += m_dx * m_speed * 0.8f;
    m_y += vy  * (m_speed * 0.9f);

    if (m_sprite) m_sprite->setFlipped(m_dx < 0);
    bounce();

    if (m_y <= 0 || m_y + getCollisionRadius()*2 >= MAXY) {
        m_phase += 3.14159f;
        m_dy = -m_dy;
    }
    if (m_x <= 0 || m_x + getCollisionRadius()*2 >= MAXX) {
        m_dx = -m_dx;
    }
}


void Angelfish::draw() const {
    if (m_sprite) m_sprite->draw(m_x, m_y);
}

//########################### SurgeonFish Implementation ######################################3
Surgeonfish::Surgeonfish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, sprite), m_tick(0) {
    do { m_dx = (rand()%3)-1; m_dy = (rand()%3)-1; } while (m_dx==0 && m_dy==0);
    normalize();
    setCollisionRadius(42);
    m_value = 3;
    m_creatureType = AquariumCreatureType::Surgeonfish;

    m_targetX = x + ((rand()%61)-30);
    m_targetY = y + ((rand()%61)-30);
}

void Surgeonfish::move() {
    const float MAXX = ofGetWidth()  - 20.0f;
    const float MAXY = ofGetHeight() - 20.0f;

    ++m_tick;
    if (m_tick % 120 == 0) {
        m_targetX = clampf(m_x + ((rand()%201)-100), 20.0f, MAXX - 20.0f);
        m_targetY = clampf(m_y + ((rand()%201)-100), 20.0f, MAXY - 20.0f);
    }

    float tx = m_targetX - (m_x + getCollisionRadius());
    float ty = m_targetY - (m_y + getCollisionRadius());
    float len = std::sqrt(tx*tx + ty*ty);
    if (len > 1e-4f) { tx/=len; ty/=len; }

    m_dx = 0.80f * m_dx + 0.20f * tx;
    m_dy = 0.80f * m_dy + 0.20f * ty;
    normalize();

    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;

    if (m_sprite) m_sprite->setFlipped(m_dx < 0);
    bounce();

    if (m_x <= 0 || m_x + getCollisionRadius()*2 >= MAXX
     || m_y <= 0 || m_y + getCollisionRadius()*2 >= MAXY) {
        m_targetX = clampf(MAXX/2.0f + ((rand()%201)-100), 20.0f, MAXX - 20.0f);
        m_targetY = clampf(MAXY/2.0f + ((rand()%201)-100), 20.0f, MAXY - 20.0f);
    }
}



void Surgeonfish::draw() const {
    if (m_sprite) m_sprite->draw(m_x, m_y);
}


// AquariumSpriteManager
AquariumSpriteManager::AquariumSpriteManager(){
    this->m_npc_fish = std::make_shared<GameSprite>("base-fish.png", 70,70);
    this->m_big_fish = std::make_shared<GameSprite>("bigger-fish.png", 120, 120);
    this->m_speed_powerup = std::make_shared<GameSprite>("powerup-speed.png", 48, 48);
     // NUEVOS PECES
    this->m_puffer_fish = std::make_shared<GameSprite>("puffer_fish.png",  92, 92);
    this->m_angelfish = std::make_shared<GameSprite>("angelfish.png",    90, 90);
    this->m_surgeonfish = std::make_shared<GameSprite>("surgeonfish.png",  96, 76);
}

std::shared_ptr<GameSprite> AquariumSpriteManager::GetSprite(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:
            return std::make_shared<GameSprite>(*this->m_big_fish);
            
        case AquariumCreatureType::NPCreature:
            return std::make_shared<GameSprite>(*this->m_npc_fish);
        case AquariumCreatureType::PufferFish:
            return std::make_shared<GameSprite>(*this->m_puffer_fish);
        case AquariumCreatureType::Angelfish:
            return std::make_shared<GameSprite>(*this->m_angelfish);
        case AquariumCreatureType::Surgeonfish:
            return std::make_shared<GameSprite>(*this->m_surgeonfish);
        default:
            return nullptr;
    }
}


// Aquarium Implementation
Aquarium::Aquarium(int width, int height, std::shared_ptr<AquariumSpriteManager> spriteManager)
    : m_width(width), m_height(height) {
        m_sprite_manager =  spriteManager;

    }



void Aquarium::addCreature(std::shared_ptr<Creature> creature) {
    creature->setBounds(m_width - 20, m_height - 20);
    m_creatures.push_back(creature);
}

void Aquarium::addAquariumLevel(std::shared_ptr<AquariumLevel> level){
    if(level == nullptr){return;} // guard to not add noise
    this->m_aquariumlevels.push_back(level);
}

void Aquarium::update() {
    // Keep aquarium bounds synced to the current window size
    m_width  = ofGetWidth();
    m_height = ofGetHeight();

    // Update bounds for every creature and move them
    for (auto& creature : m_creatures) {
        creature->setBounds(m_width - 20, m_height - 20);  // optional margin
        creature->move();  // move() already calls bounce()
    }
    maybeSpawnPowerUp();
    this->Repopulate();
}


void Aquarium::draw() const {
    for (const auto& creature : m_creatures) {
        creature->draw();
    }
    for (const auto& p : m_powerups) {
        if (p.sprite) p.sprite->draw(p.x - p.radius, p.y - p.radius);
    }
}

void Aquarium::maybeSpawnPowerUp() {
    if ((int)m_powerups.size() >= 2) return;

    ++m_powerupSpawnTimer;
    if (m_powerupSpawnTimer < 90) return;
    m_powerupSpawnTimer = 0;

    if ((rand() % 10) >= 8) return;

    PowerUpItem p;
    p.radius = 24.f;
    p.sprite = m_sprite_manager->GetPowerUpSprite(PowerUpType::SpeedBoost);

    int margin = 30;
    p.x = (float)(margin + rand() % std::max(1, getWidth()  - 2*margin));
    p.y = (float)(margin + rand() % std::max(1, getHeight() - 2*margin));

    m_powerups.push_back(std::move(p));
}

void Aquarium::removePowerUpAt(size_t idx) {
    if (idx < m_powerups.size()) m_powerups.erase(m_powerups.begin() + idx);
}

void Aquarium::removeCreature(std::shared_ptr<Creature> creature) {
    auto it = std::find(m_creatures.begin(), m_creatures.end(), creature);
    if (it != m_creatures.end()) {
        ofLogVerbose() << "removing creature " << endl;
        int selectLvl = this->currentLevel % this->m_aquariumlevels.size();
        auto npcCreature = std::static_pointer_cast<NPCreature>(creature);
        this->m_aquariumlevels.at(selectLvl)->ConsumePopulation(npcCreature->GetType(), npcCreature->getValue());
        m_creatures.erase(it);
    }
}

void Aquarium::clearCreatures() {
    m_creatures.clear();
}

std::shared_ptr<Creature> Aquarium::getCreatureAt(int index) {
    if (index < 0 || size_t(index) >= m_creatures.size()) {
        return nullptr;
    }
    return m_creatures[index];
}



void Aquarium::SpawnCreature(AquariumCreatureType type) {
    int x = rand() % this->getWidth();
    int y = rand() % this->getHeight();
    int speed = 1 + rand() % 25; // Speed between 1 and 25

    switch (type) {
        case AquariumCreatureType::NPCreature:
            this->addCreature(std::make_shared<NPCreature>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::NPCreature)));
            break;
        case AquariumCreatureType::BiggerFish:
            this->addCreature(std::make_shared<BiggerFish>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::BiggerFish)));
            break;
        case AquariumCreatureType::PufferFish:
            this->addCreature(std::make_shared<PufferFish>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::PufferFish)));
            break;
        case AquariumCreatureType::Angelfish:
            this->addCreature(std::make_shared<Angelfish>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::Angelfish)));
            break;
        case AquariumCreatureType::Surgeonfish:
            this->addCreature(std::make_shared<Surgeonfish>(x, y, speed, this->m_sprite_manager->GetSprite(AquariumCreatureType::Surgeonfish)));
            break;
        default:
            ofLogError() << "Unknown creature type to spawn!";
            break;
    }

}


// repopulation will be called from the levl class
// it will compose into aquarium so eating eats frm the pool of NPCs in the lvl class
// once lvl criteria met, we move to new lvl through inner signal asking for new lvl
// which will mean incrementing the buffer and pointing to a new lvl index
void Aquarium::Repopulate() {
    ofLogVerbose("entering phase repopulation");
    // lets make the levels circular
    int selectedLevelIdx = this->currentLevel % this->m_aquariumlevels.size();
    ofLogVerbose() << "the current index: " << selectedLevelIdx << endl;
    std::shared_ptr<AquariumLevel> level = this->m_aquariumlevels.at(selectedLevelIdx);


    if(level->isCompleted()){
        level->levelReset();
        this->currentLevel += 1;
        selectedLevelIdx = this->currentLevel % this->m_aquariumlevels.size();
        ofLogNotice()<<"new level reached : " << selectedLevelIdx << std::endl;
        level = this->m_aquariumlevels.at(selectedLevelIdx);
        this->clearCreatures();
    }

    
    // now lets find how many to respawn if needed 
    std::vector<AquariumCreatureType> toRespawn = level->Repopulate();
    ofLogVerbose() << "amount to repopulate : " << toRespawn.size() << endl;
    if(toRespawn.size() <= 0 ){return;} // there is nothing for me to do here
    for(AquariumCreatureType newCreatureType : toRespawn){
        this->SpawnCreature(newCreatureType);
    }
}

// Aquarium collision detection
std::shared_ptr<GameEvent> DetectAquariumCollisions(std::shared_ptr<Aquarium> aquarium, std::shared_ptr<PlayerCreature> player) {
    if (!aquarium || !player) return nullptr;
    
    for (int i = 0; i < aquarium->getCreatureCount(); ++i) {
        std::shared_ptr<Creature> npc = aquarium->getCreatureAt(i);
        if (npc && checkCollision(player, npc)) {
            return std::make_shared<GameEvent>(GameEventType::COLLISION, player, npc);
        }
    }
    return nullptr;
    
};

//  Imlementation of the AquariumScene

// Aquarium.cpp
void AquariumGameScene::Update() {
    std::shared_ptr<GameEvent> event;
    m_player->update();

    if (updateControl.tick()) {
        event = DetectAquariumCollisions(m_aquarium, m_player);

        if (event && event->isCollisionEvent()) {
            if (event->creatureB) {
                auto a = m_player;
                auto b = event->creatureB;

                if (a->getPower() < b->getValue()) {
                    float ar = a->getCollisionRadius();
                    float br = b->getCollisionRadius();
                    float ax = a->getX() + ar, ay = a->getY() + ar;
                    float bx = b->getX() + br, by = b->getY() + br;
                    float nx = ax - bx, ny = ay - by;
                    float dist2 = nx*nx + ny*ny;
                    float sumr  = ar + br;

                    if (dist2 < sumr*sumr) {
                        float dist = std::sqrt(std::max(1e-6f, dist2));
                        nx /= dist; ny /= dist;
                        float overlap = sumr - dist;
                        a->translate( nx * overlap * 0.60f,  ny * overlap * 0.60f);
                        b->translate(-nx * overlap * 0.40f, -ny * overlap * 0.40f);
                        a->reflect( nx, ny);
                        b->reflect(-nx,-ny);
                    }

                    a->loseLife(3*60);

                    if (a->getLives() <= 0) {
                        m_lastEvent = std::make_shared<GameEvent>(GameEventType::GAME_OVER, a, nullptr);
                        return;
                    }
                } else {
                    // STRONG ENOUGH → eat without any bounce/reflect
                    m_aquarium->removeCreature(b);
                    m_player->addToScore(1, b->getValue());
                    m_player->eatFish();

                    if (m_player->getScore() % 25 == 0) {
                        m_player->increasePower(1);
                    }
                }
            }
        }

        auto& powerUps = const_cast<std::vector<PowerUpItem>&>(m_aquarium->getPowerUps());
        for (size_t i = 0; i < powerUps.size();) {
            const PowerUpItem& p = powerUps[i];
            float ar = m_player->getCollisionRadius();
            float dx = (m_player->getX() + ar) - p.x;
            float dy = (m_player->getY() + ar) - p.y;
            float rr = ar + p.radius;
            if (dx*dx + dy*dy <= rr*rr) {
                m_player->activateSpeedBoost(2.0f, 10 * 60);
                m_aquarium->removePowerUpAt(i);
                continue;
            }
            ++i;
        }

        m_aquarium->update();
    }
}




void AquariumGameScene::Draw() {
    this->m_player->draw();
    this->m_aquarium->draw();
    this->paintAquariumHUD();

}


void AquariumGameScene::paintAquariumHUD(){
    float panelWidth = ofGetWindowWidth() - 150;
    ofDrawBitmapString("Score: " + std::to_string(this->m_player->getScore()), panelWidth, 20);
    ofDrawBitmapString("Power: " + std::to_string(this->m_player->getPower()), panelWidth, 30);
    ofDrawBitmapString("Lives: " + std::to_string(this->m_player->getLives()), panelWidth, 40);
    for (int i = 0; i < this->m_player->getLives(); ++i) {
        ofSetColor(ofColor::red);
        ofDrawCircle(panelWidth + i * 20, 50, 5);
    }
    ofSetColor(ofColor::white);
    if (this->m_player->hasSpeedBoost()) {
        int frames = this->m_player->speedBoostFramesLeft();
        int secs = (frames + 59) / 60;
        ofDrawBitmapString("Speed Boost: " + std::to_string(secs) + "s", panelWidth, 70);
    }
    ofSetColor(ofColor::white); // Reset color to white for other drawings
}

void AquariumLevel::populationReset(){
    for(auto node: this->m_levelPopulation){
        node->currentPopulation = 0; // need to reset the population to ensure they are made a new in the next level
    }
}

void AquariumLevel::ConsumePopulation(AquariumCreatureType creatureType, int power){
    for(std::shared_ptr<AquariumLevelPopulationNode> node: this->m_levelPopulation){
        ofLogVerbose() << "consuming from this level creatures" << endl;
        if(node->creatureType == creatureType){
            ofLogVerbose() << "-cosuming from type: " << AquariumCreatureTypeToString(node->creatureType) <<" , currPop: " << node->currentPopulation << endl;
            if(node->currentPopulation == 0){
                return;
            } 
            node->currentPopulation -= 1;
            ofLogVerbose() << "+cosuming from type: " << AquariumCreatureTypeToString(node->creatureType) <<" , currPop: " << node->currentPopulation << endl;
            this->m_level_score += power;
            return;
        }
    }
}

bool AquariumLevel::isCompleted(){
    return this->m_level_score >= this->m_targetScore;
}

std::vector<AquariumCreatureType> AquariumLevel::Repopulate() {
    std::vector<AquariumCreatureType> toRepopulate;
    for (const auto& node : m_levelPopulation) {
        int delta = node->population - node->currentPopulation;
        if (delta > 0) {
            for (int i = 0; i < delta; ++i) {
                toRepopulate.push_back(node->creatureType);
            }

            node->currentPopulation += delta;
        }
    }
    return toRepopulate;
}
