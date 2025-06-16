#pragma once
#include <algorithm>
#include <awc2/C/awc2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>




class CameraFPS
{
public:
	CameraFPS(
		glm::vec3 const& position = { 0.0f, 0.0f, 3.0f },
		glm::vec3 const& up 	  = { 0.0f, 1.0f, 0.0f },

		const float pitch = 0.0f,
		const float yaw   = 0.0f,
		const float vel   = 2.5f,

		const float Near = 0.1f,
		const float Far  = 100.0f,
		const float fov_degrees = 45.0f
	) :
		m_firstPos{0.0f, 0.0f},
		m_pitch{pitch},
		m_yaw{yaw},
		m_vel{vel},
		m_Near{Near},
		m_Far{Far},
		m_fov{fov_degrees},
		m_forward{0.0f},
		m_up	 {0.0f},
		m_right  {0.0f},
		m_position{position},
		m_world_up{up}
	{
		updateCameraAxes();
		m_CameraTransform  = glm::identity<glm::mat4x4>();
		m_ProjectionMatrix  = glm::identity<glm::mat4x4>();


		auto winsize = awc2getCurrentContextViewport();
		m_firstPos = glm::vec2{ winsize.x / 2, winsize.y / 2 };
		return;
	}


	void update(f32 dt)
	{
		if(awc2isMouseMoving()) {
			auto lastMousePos = awc2getMousePosition();
			m_pitch += (lastMousePos.y - m_firstPos.y) * k_MouseSensitivity;
			m_yaw   += (lastMousePos.x - m_firstPos.x) * k_MouseSensitivity;
			m_firstPos = { lastMousePos.x, lastMousePos.y };
		}
		if(awc2isMouseScrollMoving())
			m_fov += awc2getMouseScrollOffset().y * 50.0f * k_MouseSensitivity;


		m_pitch = std::clamp(m_pitch, -89.9f, 89.9f);
		m_yaw   = std::clamp(m_yaw  , -74.9f, 74.9f);
		m_fov   = std::clamp(m_fov  ,   1.0f, 45.0f);
		updateCameraAxes();

		glm::vec3 tmp{ m_vel * dt };
		if(awc2isKeyPressed(AWC2_KEYCODE_W) || awc2isKeyRepeated(AWC2_KEYCODE_W)) {
			tmp *= m_forward; 
			m_position += tmp;
		} 
		else if (awc2isKeyPressed(AWC2_KEYCODE_S) || awc2isKeyRepeated(AWC2_KEYCODE_S)) {
			tmp *= m_forward; 
			m_position -= tmp;
		} 
		else if (awc2isKeyPressed(AWC2_KEYCODE_D) || awc2isKeyRepeated(AWC2_KEYCODE_D)) {
			tmp *= m_right; 
			m_position += tmp;
		} 
		else if (awc2isKeyPressed(AWC2_KEYCODE_A) || awc2isKeyRepeated(AWC2_KEYCODE_A)) {
			tmp *= m_right; 
			m_position -= tmp;
		} 
		else if (awc2isKeyPressed(AWC2_KEYCODE_R) || awc2isKeyRepeated(AWC2_KEYCODE_R)) {
			tmp *= m_up; 
			m_position += tmp;
		} 
		else if (awc2isKeyPressed(AWC2_KEYCODE_F) || awc2isKeyRepeated(AWC2_KEYCODE_F)) {
			tmp *= m_up; 
			m_position -= tmp;
		}


		tmp = m_position + m_forward;
		m_CameraTransform = glm::lookAt(m_position, tmp, m_up);
		
		auto winsize	   = awc2getCurrentContextViewport();
		f32 aspectRatio    = __scast(f32, winsize.x) / winsize.y;
		m_ProjectionMatrix = glm::perspective(glm::radians(m_fov), aspectRatio, m_Near, m_Far);
		return;
	}


	__force_inline void updateProjectionParameters(
		f32 fov  = 60.0f, 
		f32 near = 0.1f, 
		f32 far  = 100.0f
	) {
		auto winsize    = awc2getCurrentContextViewport();
		f32 aspectRatio = __scast(f32, winsize.x) / winsize.y;
		m_Near = near;
		m_Far  = far;
		m_fov  = fov;
		m_ProjectionMatrix = glm::perspective(glm::radians(m_fov), aspectRatio, m_Near, m_Far);
		return;
	}


	__force_inline void updateCameraSpeed(f32 cameraSpeed) {
		m_vel = cameraSpeed;
		return;
	}


	__force_inline glm::mat4x4 const& getView() 	  const { return m_CameraTransform;  }
	__force_inline glm::mat4x4 const& getProjection() const { return m_ProjectionMatrix; }
	__force_inline glm::vec3   const& getPosition()   const { return m_position; }
	__force_inline glm::vec3   getDirection()  		  const { return m_position + m_forward; }


private:
	void updateCameraAxes()
	{
		m_forward = glm::vec3{
			cosf(m_yaw) * cosf(m_pitch),
			sinf(m_pitch),
			sinf(m_yaw) * cosf(m_pitch)
		};
		m_forward = glm::normalize(m_forward);

		
		m_right = glm::cross(m_forward, m_world_up);
		m_right = glm::normalize(m_right);

		m_up = glm::cross(m_right, m_forward);
		m_up = glm::normalize(m_up);
		return;
	}


private:
	static constexpr f32 k_MouseSensitivity = 10e-4f;
	glm::vec2 m_firstPos;
	
	f32 m_pitch;
	f32 m_yaw;
	f32 m_vel;

	f32 m_Near;
	f32 m_Far;
	f32 m_fov;

	glm::vec3 m_forward {0.0f, 0.0f, -1.0f};
	glm::vec3 m_up      {0.0f};
	glm::vec3 m_right   {0.0f};
	
	glm::vec3   m_position;
	glm::vec3   m_world_up;
	glm::mat4x4 m_CameraTransform;  // view matrix
	glm::mat4x4 m_ProjectionMatrix; // projection matrix.
};