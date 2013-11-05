/*
 * Copyright 2011-2013 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "window/RenderWindow.h"

#include <algorithm>

#include <boost/foreach.hpp>

void RenderWindow::addRenderListener(RendererListener * listener) {
	m_renderListeners.push_back(listener);
}

void RenderWindow::removeRenderListener(RendererListener * listener) {
	m_renderListeners.erase(std::remove(m_renderListeners.begin(), m_renderListeners.end(),
	                        listener), m_renderListeners.end());
}

void RenderWindow::onRendererInit() {
	BOOST_FOREACH(RendererListener * listener, m_renderListeners) {
		listener->onRendererInit(*this);
	}
}

void RenderWindow::onRendererShutdown() {
	BOOST_FOREACH(RendererListener * listener, m_renderListeners) {
		listener->onRendererShutdown(*this);
	}
}
