/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	
	class timer;
	extern "C" void timer_tick_callback(timer* an_owner);
	extern "C" void timer_qfn_callback(timer* a_timer);

	
	/// The timer class allows you to schedule a function to be called in the future using Max's scheduler.
	/// Note: the name `timer` was chosen instead of `clock` because of the use of the type is `clock` is ambiguous on the Mac OS
	/// when not explicitly specifying the `c74::min` namespace.
	///
	///	@seealso	#time_value
	/// @seealso	#queue
	/// @seealso	#fifo
	
	class timer {
	public:


		/// Options that control the behavior of the timer.

		enum class options {
			deliver_on_scheduler,	/// The default behavior delivers events on Max's scheduler thread
			defer_delivery			/// Defers events from the scheduler to Max's main thread
		};


		/// Create a timer.
		/// @param	an_owner	The owning object for the timer. Typically you will pass `this`.
		/// @param	a_function	A function to be executed when the timer is called.
		///						Typically the function is defined using a C++ lambda with the #MIN_FUNCTION signature.
		/// @param	options		Optional argument to alter the delivery from the scheduler thread to the main thread.

		timer(object_base* an_owner, function a_function, options options = options::deliver_on_scheduler)
		: m_owner		{ an_owner }
		, m_function	{ a_function }
		{
			m_instance = max::clock_new(this, reinterpret_cast<max::method>(timer_tick_callback));
			if (options == options::defer_delivery)
				m_qelem = max::qelem_new(this, reinterpret_cast<max::method>(timer_qfn_callback));
		}

		/// Create a timer.
		/// @param	an_owner	The owning object for the timer. Typically you will pass `this`.
		/// @param	options		Optional argument to alter the delivery from the scheduler thread to the main thread.
		/// @param	a_function	A function to be executed when the timer is called.
		///						Typically the function is defined using a C++ lambda with the #MIN_FUNCTION signature.

		timer(object_base* an_owner, options options, function a_function)
		: timer(an_owner, a_function, options)
		{}


		/// Destroy a timer.
		
		~timer() {
			object_free(m_instance);
			if (m_qelem)
				max::qelem_free(m_qelem);
		}


		// Timers cannot be copied.
		// If they are then the ownership of the internal t_clock becomes ambiguous.

		timer(const timer&) = delete;
		timer& operator = (const timer& value) = delete;


		/// Set the timer to fire after a specified delay.
		/// When the timer fires its function will be executed.
		/// @param	duration_in_ms	The length of the delay (from "now") before the timer fires.

		void delay(double duration_in_ms) {
			clock_fdelay(m_instance, duration_in_ms);
		}
		

		/// Stop a timer that has been previously set using the delay() call.

		void stop() {
			if (m_instance)
				clock_unset(m_instance);
		}


		/// Execute the timer's function immediately / synchronously.
		
		void tick() {
			atoms a;
			m_function(a);
		}


		/// Determine if the timer executes it's function by deferring.
		/// @return		True if the timer defers to the main thread. Otherwise false.

		bool should_defer() {
			return m_qelem;
		}


		/// post information about the timer to the console
		// also serves the purpose of eliminating warnings about m_owner being unused
		void post() {
			std::cout << m_instance << &m_function << m_owner << std::endl;
		}
		
		
	private:
		object_base*	m_owner;
		function		m_function;
		max::t_clock*	m_instance { nullptr };
		max::t_qelem*	m_qelem { nullptr };

		friend void timer_tick_callback(timer* an_owner);
		void defer() {
			max::qelem_set(m_qelem);
		}
	};


}} // namespace c74::min
