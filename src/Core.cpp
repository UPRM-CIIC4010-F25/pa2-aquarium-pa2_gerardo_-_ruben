#include "Core.h"


// Creature Inherited Base Behavior
void Creature::setBounds(int w, int h) { m_width = w; m_height = h; }
void Creature::normalize() {
    float length = std::sqrt(m_dx * m_dx + m_dy * m_dy);
    if (length != 0) {
        m_dx /= length;
        m_dy /= length;
    }
}

void Creature::bounce() {
    // Use collision circle diameter if no sprite size is available
    const float sw = (m_collisionRadius > 0.0f) ? (m_collisionRadius * 2.0f) : 20.0f;
    const float sh = (m_collisionRadius > 0.0f) ? (m_collisionRadius * 2.0f) : 20.0f;

    // Use the bounds set by Aquarium::setBounds; if zero, fall back to current window size
    const float limitW = (m_width  > 0.0f) ? m_width  : static_cast<float>(ofGetWidth());
    const float limitH = (m_height > 0.0f) ? m_height : static_cast<float>(ofGetHeight());

    const float maxX = std::max(0.0f, limitW - sw);
    const float maxY = std::max(0.0f, limitH - sh);

    // Horizontal
    if (m_x < 0.0f) {
        m_x = 0.0f;
        m_dx = -m_dx;
    } else if (m_x > maxX) {
        m_x = maxX;
        m_dx = -m_dx;
    }

    // Vertical
    if (m_y < 0.0f) {
        m_y = 0.0f;
        m_dy = -m_dy;
    } else if (m_y > maxY) {
        m_y = maxY;
        m_dy = -m_dy;
    }
}



void GameEvent::print() const {
        
        switch (type) {
            case GameEventType::NONE:
                ofLogVerbose() << "No event." << std::endl;
                break;
            case GameEventType::COLLISION:
                ofLogVerbose() << "Collision event between creatures at (" 
                << creatureA->getX() << ", " << creatureA->getY() << ") and ("
                << creatureB->getX() << ", " << creatureB->getY() << ")." << std::endl;
                break;
            case GameEventType::CREATURE_ADDED:
                ofLogVerbose() << "Creature added at (" 
                << creatureA->getX() << ", " << creatureA->getY() << ")." << std::endl;
                break;
            case GameEventType::CREATURE_REMOVED:
                ofLogVerbose() << "Creature removed at (" 
                << creatureA->getX() << ", " << creatureA->getY() << ")." << std::endl;
                break;
            case GameEventType::GAME_OVER:
                ofLogVerbose() << "Game Over event." << std::endl;
                break;
            case GameEventType::NEW_LEVEL:
                ofLogVerbose() << "New Game level" << std::endl;
            default:
                ofLogVerbose() << "Unknown event type." << std::endl;
                break;
        }
};

// collision detection between two creatures
bool checkCollision(std::shared_ptr<Creature> a, std::shared_ptr<Creature> b) {
    if (!a || !b) return false;
    const float ar = a->getCollisionRadius();
    const float br = b->getCollisionRadius();

    const float ax = a->getX() + ar;
    const float ay = a->getY() + ar;
    const float bx = b->getX() + br;
    const float by = b->getY() + br;

    const float dx = ax - bx;
    const float dy = ay - by;
    const float rr = ar + br;

    return (dx*dx + dy*dy) <= (rr * rr);
}




std::string GameSceneKindToString(GameSceneKind t) {
    switch (t) {
        case GameSceneKind::GAME_INTRO:    return "GAME_INTRO";
        case GameSceneKind::AQUARIUM_GAME: return "AQUARIUM_GAME";
        case GameSceneKind::GAME_OVER:     return "GAME_OVER";
        default:                           return "UNKNOWN_SCENE";
    }
}

std::shared_ptr<GameScene> GameSceneManager::GetScene(string name){
    if(!this->HasScenes()){return nullptr;}
    for(std::shared_ptr<GameScene> scene : this->m_scenes){
        if(scene->GetName() == name){
            return scene;
        }
    }
    return nullptr;
}

void GameSceneManager::Transition(string name){
    if(!this->HasScenes()){return;} // no need to do anything if nothing inside
    std::shared_ptr<GameScene> newScene = this->GetScene(name);
    if(newScene == nullptr){return;} // i dont have the scene so time to leave
    if(newScene->GetName() == this->m_active_scene->GetName()){return;} // another do nothing since active scene is already pulled
    this->m_active_scene = newScene; // now we keep it since this is a valid transition
    return;
}

void GameSceneManager::AddScene(std::shared_ptr<GameScene> newScene){
    if(this->GetScene(newScene->GetName()) != nullptr){
        return; // this scene already exist and shouldnt be added again
    }
    this->m_scenes.push_back(newScene);
    if(m_active_scene == nullptr){
        this->m_active_scene = newScene; // need to place in active scene as its the only one in existance right now
    }
    return;
}

std::shared_ptr<GameScene> GameSceneManager::GetActiveScene(){
    return this->m_active_scene;
}

string GameSceneManager::GetActiveSceneName(){
    if(this->m_active_scene == nullptr){return "";} // something to handle missing activate scenes
    return this->m_active_scene->GetName();
}

void GameSceneManager::UpdateActiveScene(){
    if(!this->HasScenes()){return;} // make sure we have a scene before we try to paint
    this->m_active_scene->Update();

}

void GameSceneManager::DrawActiveScene(){
    if(!this->HasScenes()){return;} // make sure we have something before Drawing it
    this->m_active_scene->Draw();
}


void GameIntroScene::Update(){

}

void GameIntroScene::Draw(){
    this->m_banner->draw(0,0);
}

void GameOverScene::Update(){

}

void GameOverScene::Draw(){
    ofBackgroundGradient(ofColor::red, ofColor::black);
    this->m_banner->draw(0,0);

}