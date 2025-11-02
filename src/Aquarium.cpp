#include "Aquarium.h"
#include <cstdlib>


string AquariumCreatureTypeToString(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish: return "BiggerFish";
        case AquariumCreatureType::NPCreature: return "BaseFish";
        case AquariumCreatureType::ClownFish:  return "ClownFish";
        case AquariumCreatureType::BlueTang:   return "BlueTang";    
        default: return "UknownFish";
    }
}


// PlayerCreature Implementation
PlayerCreature::PlayerCreature(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: Creature(x, y, speed, 35.0f, 1, sprite) {}


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

void PlayerCreature::update() {
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

// ===================== ClownFish =====================
ClownFish::ClownFish(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, std::move(sprite)) {
    m_creatureType = AquariumCreatureType::ClownFish;
    homeX = x; 
    homeY = y;
    setCollisionRadius(50);
    setHidden(true);            // empieza escondido
    scheduleNextCuckoo();       // programa la primera salida
}
void ClownFish::scheduleNextCuckoo() {
    nextCuckooAt = ofGetElapsedTimef() + ofRandom(intervalMin, intervalMax);
}

void ClownFish::setHidden(bool h) {
    hidden = h;
    if (hidden) {
        // “se guarda” en la anémona
        m_x = homeX; 
        m_y = homeY;
        m_dx = 0; m_dy = 0;
        setCollisionRadius(1);     // prácticamente sin colisión
    } else {
        setCollisionRadius(50);
    }
}
void ClownFish::move() {
    float now = ofGetElapsedTimef();

    switch (state) {
    case State::Hidden: {
        // Espera hasta la hora de salir
        if (now >= nextCuckooAt) {
            // Comienza a emerger en dirección ligeramente hacia arriba
            angle = ofRandom(-0.6f, 0.6f); // rumbo inicial lateral
            state = State::Emerging;
            stateEnd = now + emergeDur;
            setHidden(false);
        }
        return; // nada más que hacer oculto
    }

    case State::Emerging: {
        // Avanza suavemente fuera de la anémona
        float vx = cosf(angle);
        float vy = -0.4f + sinf(angle*0.6f)*0.1f; // sesgo hacia arriba
        float len = std::max(1e-6f, sqrtf(vx*vx + vy*vy));
        m_dx = vx/len; m_dy = vy/len;
        m_sprite->setFlipped(m_dx < 0.f);

        m_x += m_dx * m_speed;
        m_y += m_dy * m_speed;

        if (now >= stateEnd) {
            state = State::Roam;
            stateEnd = now + ofRandom(roamDurMin, roamDurMax);
        }
        bounce();
        break;
    }

    case State::Roam: {
        // Nada libre (random walk suave)
        angle += ofRandom(-0.06f, 0.06f);
        float vx = cosf(angle) + ofRandom(-0.04f, 0.04f);
        float vy = sinf(angle) + 0.9f * sinf(TWO_PI * 0.12f * now) + ofRandom(-0.02f, 0.02f);
        float len = std::max(1e-6f, sqrtf(vx*vx + vy*vy));
        m_dx = vx/len; m_dy = vy/len;
        m_sprite->setFlipped(m_dx < 0.f);

        m_x += m_dx * m_speed;
        m_y += m_dy * m_speed;

        if (now >= stateEnd) {
            state = State::Returning;
        }
        bounce();
        break;
    }

    case State::Returning: {
        // Vuelve directo a la anémona
        float hx = homeX - m_x, hy = homeY - m_y;
        float d = std::max(1e-6f, sqrtf(hx*hx + hy*hy));
        m_dx = hx/d; m_dy = hy/d;
        m_sprite->setFlipped(m_dx < 0.f);

        m_x += m_dx * (m_speed * 1.2f); // un pelín más rápido retornando
        m_y += m_dy * (m_speed * 1.2f);

        if (d < 16.f) {
            // llegó
            state = State::Rest;
            stateEnd = now + ofRandom(restMin, restMax);
            // “encaja” exactamente en el hogar
            m_x = homeX; m_y = homeY;
        }
        bounce();
        break;
    }

    case State::Rest: {
        // Descansa dentro/encima de la anémona con una leve mecedura
        m_x = homeX + sinf(now*2.1f)*1.5f;
        m_y = homeY + cosf(now*2.3f)*1.5f;
        if (now >= stateEnd) {
            state = State::Hidden;
            setHidden(true);
            scheduleNextCuckoo();
        }
        break;
    }
    }
}



void ClownFish::draw() const {
    if (hidden) return;
    m_sprite->setFlipped(m_dx < 0.f);
    m_sprite->draw(m_x, m_y);
}



// ===================== BlueTang =====================
BlueTang::BlueTang(float x, float y, int speed, std::shared_ptr<GameSprite> sprite)
: NPCreature(x, y, speed, std::move(sprite)) {
    m_creatureType = AquariumCreatureType::BlueTang;
    heading = ofRandom(-PI, PI);
    desiredHeading = heading;
}

void BlueTang::move() {
    //cambia lentamente el rumbo deseado (random walk)
    desiredHeading += ofRandom(-0.02f, 0.02f);

    // limita cuánto puede girar por frame
    float delta = ofWrapRadians(desiredHeading - heading);
    delta = ofClamp(delta, -maxTurnRate, maxTurnRate);
    heading = ofWrapRadians(heading + delta);

    
    t += 0.03f;
    float vx = cosf(heading) * baseSpeed;
    float vy = sinf(heading) * baseSpeed + sineAmp * sinf(TWO_PI * 1.2f * t);

    m_dx = vx;
    m_dy = vy;
    normalize();              // asegura que (dx,dy) sea unitario para escalar con m_speed
    m_sprite->setFlipped(m_dx < 0.f);

    m_x += m_dx * m_speed;
    m_y += m_dy * m_speed;

    bounce();
}

void BlueTang::draw() const {
    m_sprite->draw(m_x, m_y);
}



// AquariumSpriteManager
AquariumSpriteManager::AquariumSpriteManager(){
    this->m_npc_fish = std::make_shared<GameSprite>("base-fish.png", 70,70);
    this->m_big_fish = std::make_shared<GameSprite>("bigger-fish.png", 120, 120);
   // NUEVOS:
    this->m_clown_fish = std::make_shared<GameSprite>("clown-fish.png", 60, 60);

    this->m_blue_tang  = std::make_shared<GameSprite>("blue-tang.png", 70, 70);



}

std::shared_ptr<GameSprite> AquariumSpriteManager::GetSprite(AquariumCreatureType t){
    switch(t){
        case AquariumCreatureType::BiggerFish:
            return std::make_shared<GameSprite>(*this->m_big_fish);   
        case AquariumCreatureType::NPCreature:
            return std::make_shared<GameSprite>(*this->m_npc_fish);
        case AquariumCreatureType::ClownFish:
            return std::make_shared<GameSprite>(*m_clown_fish);
        case AquariumCreatureType::BlueTang:
            return std::make_shared<GameSprite>(*m_blue_tang);

        default:
            return nullptr;
    }
}


// Aquarium Implementation
Aquarium::Aquarium(int width, int height, std::shared_ptr<AquariumSpriteManager> spriteManager)
    : m_width(width), m_height(height) {
        m_sprite_manager =  spriteManager;
    }

void Aquarium::initSeafloor() { // 
    if (!m_anemoneImg.isAllocated()) {
    // Coloca tu PNG en bin/data/, por ejemplo: "anemone.png"
    bool ok = m_anemoneImg.load("anemone.png");
    ofLogNotice() << "[ANEMONE] load: " << ok;
    ofEnableAlphaBlending();
    }




    m_anemones.clear();

    m_sandHeight = std::max(80.f, ofGetHeight() * 0.18f);
    float baseY  = ofGetHeight() - m_sandHeight + 12.f;

    float marginX = 80.f; // separadas de los bordes
    m_anemones.push_back({ marginX,               baseY, 70.f });          // izquierda
    m_anemones.push_back({ ofGetWidth()-marginX,  baseY, 70.f });          // derecha

    m_anemonesInit = true;
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

    if (!m_anemonesInit) initSeafloor();

    // si cambia el tamaño de ventana, re-crear arena + anémonas
    if (m_lastW != m_width || m_lastH != m_height) {
        initSeafloor();
        m_lastW = m_width;
        m_lastH = m_height;
    }


    // Update bounds for every creature and move them
    for (auto& creature : m_creatures) {
        creature->setBounds(m_width - 20, m_height - 20);  // optional margin
        creature->move();  // move() already calls bounce()
    }

    this->Repopulate();
}


void Aquarium::draw() const {

    // --- Anémonas (imagen o fallback) ---
    for (const auto& a : m_anemones) {
        float w = m_anemoneW;
        float h = m_anemoneH;

        float drawX = a.x - w * 0.5f;   // centrar
        float drawY = a.y - h;          // base apoyada en arena

        if (m_anemoneImg.isAllocated()) {
            ofSetColor(255);            // sin tinte
            m_anemoneImg.draw(drawX, drawY, w, h);
        } else {
            ofSetColor(240,120,140);
            ofDrawCircle(a.x, a.y, 12);
            ofSetColor(255);
        }
    }
    ofPopStyle();

    // --- Peces ---
    for (const auto& creature : m_creatures) {
        creature->draw();
    }
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
    int speed = 1 + rand() % 25;

    switch (type) {

        case AquariumCreatureType::NPCreature: {
            addCreature(std::make_shared<NPCreature>(x, y, speed, m_sprite_manager->GetSprite(AquariumCreatureType::NPCreature)));
            break;
        }

        case AquariumCreatureType::BiggerFish: {
            addCreature(std::make_shared<BiggerFish>(x, y, speed, m_sprite_manager->GetSprite(AquariumCreatureType::BiggerFish)));
            break;
        }

        case AquariumCreatureType::ClownFish: {
            if (!m_anemonesInit) initSeafloor();
            const Anemone& A = m_anemones[ rand() % m_anemones.size() ];

            float anemoneTopY   = A.y - m_anemoneH;
            float tentacleBandY = anemoneTopY + m_anemoneH * ofRandom(0.35f, 0.50f);

            int cx = (int)(A.x + ofRandom(-24.f, 24.f));
            int cy = (int)(tentacleBandY + ofRandom(-8.f, 8.f));
            int sp = 2 + rand() % 6; // 2..7, más natural

            addCreature(std::make_shared<ClownFish>(cx, cy, sp, m_sprite_manager->GetSprite(AquariumCreatureType::ClownFish)));
            break;
        }

        


        case AquariumCreatureType::BlueTang: {    
            addCreature(std::make_shared<BlueTang>(x, y, speed, m_sprite_manager->GetSprite(AquariumCreatureType::BlueTang)));
            break;
        }

        default: {
            ofLogError() << "Unknown creature type to spawn!";
            break;
        }
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

void AquariumGameScene::Update(){
    std::shared_ptr<GameEvent> event;

    this->m_player->update();

    if (this->updateControl.tick()) {
        event = DetectAquariumCollisions(this->m_aquarium, this->m_player);

        if (event != nullptr && event->isCollisionEvent()) {
            ofLogVerbose() << "Collision detected between player and NPC!" << std::endl;

            if (event->creatureB != nullptr) {
                event->print();
                {
                    auto a = this->m_player;
                    auto b = event->creatureB;

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
                }
                if (this->m_player->getPower() < event->creatureB->getValue()) {
                    ofLogNotice() << "Player is too weak to eat the creature!" << std::endl;
                    this->m_player->loseLife(3*60); // 3 seconds @60fps
                    {
                        float ar = this->m_player->getCollisionRadius();
                        float br = event->creatureB->getCollisionRadius();
                        float ax = this->m_player->getX() + ar, ay = this->m_player->getY() + ar;
                        float bx = event->creatureB->getX() + br, by = event->creatureB->getY() + br;
                        float kx = ax - bx, ky = ay - by;
                        float klen = std::sqrt(kx*kx + ky*ky);
                        if (klen > 1e-6f) { kx /= klen; ky /= klen; }
                        this->m_player->translate(kx * 12.0f, ky * 12.0f);
                    }

                    if (this->m_player->getLives() <= 0) {
                        this->m_lastEvent = std::make_shared<GameEvent>(GameEventType::GAME_OVER, this->m_player, nullptr);
                        return;
                    }
                } else {
                    this->m_aquarium->removeCreature(event->creatureB);
                    this->m_player->addToScore(1, event->creatureB->getValue());
                    if (this->m_player->getScore() % 25 == 0) {
                        this->m_player->increasePower(1);
                        ofLogNotice() << "Player power increased to " << this->m_player->getPower() << "!" << std::endl;
                    }
                }

            } else {
                ofLogError() << "Error: creatureB is null in collision event." << std::endl;
            }
        }
        this->m_aquarium->update();
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




std::vector<AquariumCreatureType> Level_0::Repopulate() {
    std::vector<AquariumCreatureType> toRepopulate;
    for(std::shared_ptr<AquariumLevelPopulationNode> node : this->m_levelPopulation){
        int delta = node->population - node->currentPopulation;
        ofLogVerbose() << "to Repopulate :  " << delta << endl;
        if(delta >0){
            for(int i = 0; i<delta; i++){
                toRepopulate.push_back(node->creatureType);
            }
            node->currentPopulation += delta;
        }
    }
    return toRepopulate;

}

std::vector<AquariumCreatureType> Level_1::Repopulate() {
    std::vector<AquariumCreatureType> toRepopulate;
    for(std::shared_ptr<AquariumLevelPopulationNode> node : this->m_levelPopulation){
        int delta = node->population - node->currentPopulation;
        if(delta >0){
            for(int i=0; i<delta; i++){
                toRepopulate.push_back(node->creatureType);
            }
            node->currentPopulation += delta;
        }
    }
    return toRepopulate;
}

std::vector<AquariumCreatureType> Level_2::Repopulate() {
    std::vector<AquariumCreatureType> toRepopulate;
    for(std::shared_ptr<AquariumLevelPopulationNode> node : this->m_levelPopulation){
        int delta = node->population - node->currentPopulation;
        if(delta >0){
            for(int i=0; i<delta; i++){
                toRepopulate.push_back(node->creatureType);
            }
            node->currentPopulation += delta;
        }
    }
    return toRepopulate;
}
