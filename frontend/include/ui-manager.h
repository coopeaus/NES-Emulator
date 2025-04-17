#pragma once
#include <vector>
#include <memory>

// Forward declarations for our components.
class UIComponent;
class MainMenuBar;
class OverlayWindow;
class DemoWindow;
class DebuggerWindow;
class Renderer;

class UIManager
{
public:
  bool willRender = true;

  UIManager( Renderer *renderer );

  // Rule of 5
  ~UIManager() = default;
  UIManager( const UIManager & ) = delete;
  UIManager &operator=( const UIManager & ) = delete;
  UIManager( UIManager && ) = delete;
  UIManager &operator=( UIManager && ) = delete;

  // Render all UI elements
  void Render();

  // Templated accessor to retrieve a component of a given type.
  // Usage
  // if (auto demoWin = UIManagerInstance.GetComponent<DemoWindow>()) {}
  template <typename T> T *GetComponent()
  {
    for ( auto &comp : _components ) {
      if ( T *result = dynamic_cast<T *>( comp.get() ) ) {
        return result;
      }
    }
    return nullptr;
  }

private:
  std::vector<std::unique_ptr<UIComponent>> _components;

  // Helper template to add a component.
  template <typename T, typename... Args> T *AddComponent( Args &&...args )
  {
    auto comp = std::make_unique<T>( std::forward<Args>( args )... );
    T   *ptr = comp.get();
    _components.push_back( std::move( comp ) );
    return ptr;
  }
};
