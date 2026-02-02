#include "Camera.h"
#include "Config.h"
#include <algorithm>

Camera::Camera(float width, float height)
        : mWidth(width), mHeight(height) {}

void Camera::update(sf::View& view, sf::Vector2f carPos, sf::Vector2f trackSize) const {
    float minX = mWidth / 2.f;
    float maxX = trackSize.x - mWidth / 2.f;
    float minY = mHeight / 2.f;
    float maxY = trackSize.y - mHeight / 2.f;

    // Clamp car position inside track bounds
    carPos.x = std::max(minX, std::min(carPos.x, maxX));
    carPos.y = std::max(minY, std::min(carPos.y, maxY));

    view.setCenter(carPos); // https://www.sfml-dev.org/documentation/3.0.0/classsf_1_1View.html#a32f9cfaeb3c753a545fbe54b7d882ea5
}

void Camera::handleResize(const sf::Event::Resized& resized) {
    mWidth = resized.size.x / Config::CAMERA_RESIZE_FACTOR;
    mHeight = resized.size.y / Config::CAMERA_RESIZE_FACTOR;
}
