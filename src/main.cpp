#include "platform/platform.hpp"
#include <Geode/modify/CCKeyboardDispatcher.hpp>
#include <Geode/modify/AchievementNotifier.hpp>
#include <Geode/modify/CCDirector.hpp>
#include <Geode/modify/CCEGLView.hpp>
#include <Geode/modify/CCNode.hpp>
#include "DevTools.hpp"
#include <imgui.h>
#include "ImGui.hpp"

using namespace geode::prelude;

class $modify(CCNode) {
    void sortAllChildren() override {
        if (DevTools::get()->shouldOrderChildren()) {
            CCNode::sortAllChildren();
        }
    }
};

// todo: use shortcuts api once Geode has those
class $modify(CCKeyboardDispatcher) {
    bool dispatchKeyboardMSG(enumKeyCodes key, bool down, bool arr) {
        if (down && (key == KEY_F11 GEODE_MACOS(|| key == KEY_F10))) {
            DevTools::get()->toggle();
            return true;
        }
        return CCKeyboardDispatcher::dispatchKeyboardMSG(key, down, arr);
    }
};

#ifdef GEODE_IS_MOBILE
// lol
#include <Geode/modify/MenuLayer.hpp>
class $modify(DTMenuLayer, MenuLayer) {
    void onDevTools(CCObject*) {
        DevTools::get()->toggle();
    }
    bool init() {
        if (!MenuLayer::init()) {
            return false;
        }

        auto btn = CCMenuItemSpriteExtra::create(
            CircleButtonSprite::createWithSprite(
                "devtools.png"_spr, 1, CircleBaseColor::Green,
                CircleBaseSize::MediumAlt),
            this, menu_selector(DTMenuLayer::onDevTools));
        btn->setID("devtools"_spr);
        auto menu = this->getChildByID("bottom-menu");
        menu->addChild(btn);
        menu->updateLayout();
        return true;
    }
};

#endif

class $modify(CCDirector) {
    void willSwitchToScene(CCScene* scene) {
        CCDirector::willSwitchToScene(scene);
        DevTools::get()->sceneChanged();
    }

    void drawScene() {
        if (!DevTools::get()->shouldUseGDWindow()) {
            return CCDirector::drawScene();
        }
        
        DevTools::get()->setup();

        static GLRenderCtx* gdTexture = nullptr;

        if (!DevTools::get()->shouldPopGame()) {
            if (gdTexture) {
                delete gdTexture;
                gdTexture = nullptr;
            }
            shouldPassEventsToGDButTransformed() = false;
            CCDirector::drawScene();
            return;
        }

        if (shouldUpdateGDRenderBuffer()) {
            if (gdTexture) {
                delete gdTexture;
                gdTexture = nullptr;
            }
            shouldUpdateGDRenderBuffer() = false;
        }

        auto winSize = this->getOpenGLView()->getViewPortRect() * geode::utils::getDisplayFactor();
        if (!gdTexture) {
            gdTexture = new GLRenderCtx({ winSize.size.width, winSize.size.height });
        }

        if (!gdTexture->begin()) {
            delete gdTexture;
            gdTexture = nullptr;
            CCDirector::drawScene();
            DevTools::get()->render(nullptr);
            return;
        }
        CCDirector::drawScene();
        gdTexture->end();

        DevTools::get()->render(gdTexture);

        // if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        //     auto backup_current_context = this->getOpenGLView()->getWindow();
        //     ImGui::UpdatePlatformWindows();
        //     ImGui::RenderPlatformWindowsDefault();
        //     glfwMakeContextCurrent(backup_current_context);
        // }
    }
};

class $modify(CCEGLView) {
    // this is needed for popout mode because we need to render after gd has rendered,
    // but before the buffers have been swapped, which is not possible with just a
    // CCDirector::drawScene hook.
    void swapBuffers() {
        if (!DevTools::get()->shouldUseGDWindow() || !DevTools::get()->shouldPopGame()) {
            DevTools::get()->setup();
            DevTools::get()->render(nullptr);
        }
        CCEGLView::swapBuffers();
    }
};
