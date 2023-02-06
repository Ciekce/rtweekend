#include "camera.h"

namespace cpurt
{
	Camera::Camera(u32 width, u32 height, f32 fovY, f32 aperture, f32 focalLength)
		: m_width{width},
		  m_height{height},
		  m_fovY{fovY},
		  m_aperture{aperture},
		  m_focalLength{focalLength} {}

	void Camera::update()
	{
		m_invSize = {1.0F / static_cast<f32>(m_width - 1),
			1.0F / static_cast<f32>(m_height - 1)};

		const auto aspect = static_cast<f32>(m_width) / static_cast<f32>(m_height);

		const auto vh = 2.0F * glm::tan(glm::radians(m_fovY) / 2.0F);
		const auto vw = vh * aspect;

		m_w = glm::normalize(m_pos - m_target);
		m_u = glm::normalize(glm::cross(glm::vec3{0.0F, 1.0F, 0.0F}, m_w));
		m_v = glm::cross(m_w, m_u);

		m_horizontal = m_focalLength * vw * m_u;
		m_vertical = m_focalLength * vh * m_v;

		m_lowerLeft = -m_horizontal / 2.0F - m_vertical / 2.0F - m_focalLength * m_w;

		m_lensRadius = m_aperture / 2.0F;
	}
}
