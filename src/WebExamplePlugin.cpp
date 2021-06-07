/*
 * Hip-Hop / High Performance Hybrid Audio Plugins
 * Copyright (C) 2021 Luciano Iam <oss@lucianoiam.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "WebExamplePlugin.hpp"

USE_NAMESPACE_DISTRHO

Plugin* DISTRHO::createPlugin()
{
    return new WebExamplePlugin;
}

WebExamplePlugin::WebExamplePlugin()
    : Plugin(0, 0, 0)
{}

void WebExamplePlugin::initParameter(uint32_t index, Parameter& parameter)
{
    // unused
    (void)index;
    (void)parameter;
}

float WebExamplePlugin::getParameterValue(uint32_t index) const
{
    return 0;

    // unused
    (void)index;
}

void WebExamplePlugin::setParameterValue(uint32_t index, float value)
{
    // unused
    (void)index;
    (void)value;
}

void WebExamplePlugin::run(const float** inputs, float** outputs, uint32_t frames)
{
    memcpy(outputs[0], inputs[0], frames * sizeof(float));
}
