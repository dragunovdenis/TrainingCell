#pragma once
#include <memory>
#include <mutex>

namespace TrainingCellDll
{
	/// <summary>
	///	Pointer to a "locked" resource
	/// </summary>
	template <class T>
	class LockedResourcePtr
	{
		T* _ptr{};

		std::unique_lock<std::mutex> _lock{};

	public:
		/// <summary>
		///	Constructor
		/// </summary>
		LockedResourcePtr(std::mutex& mtx, T* ptr) : _ptr(ptr), _lock(mtx)
		{}

		/// <summary>
		///	Access to the underlying pointer
		/// </summary>
		T* ptr()
		{
			return _ptr;
		}

		/// <summary>
		///	Read-only access to the underlying pinter
		/// </summary>
		const T* ptr() const
		{
			return _ptr;
		}
	};

	/// <summary>
	///	A thread-safe wrapper to data
	/// </summary>
	template <class T>
	class LockableResource
	{
		/// <summary>
		///	Pointer to the resource
		/// </summary>
		std::unique_ptr<T> _resource_ptr{};

		/// <summary>
		///	Mutex to control resource access
		/// </summary>
		std::mutex _mtx{};

		/// <summary>
		///	A flack to track state of the mutex
		/// </summary>
		bool _is_locked{};

	public:
		/// <summary>
		///	Constructor
		/// </summary>
		template <class... Types>
		LockableResource(Types&&... args)
		{
			_resource_ptr = std::make_unique<T>(std::forward<Types>(args)...);
		}

		template <class R>
		LockableResource(std::unique_ptr<R>&& u_ptr)
		{
			_resource_ptr = std::move(u_ptr);
		}

		/// <summary>
		///	Locks the resource
		/// </summary>
		LockedResourcePtr<T> lock()
		{
			if (_resource_ptr.get() == nullptr)
				throw std::exception("Attempt to access not/de-allocated resource");

			return LockedResourcePtr<T>{_mtx, _resource_ptr.get()};
		}
	};
}
