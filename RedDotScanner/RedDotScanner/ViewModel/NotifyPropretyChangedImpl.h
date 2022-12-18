#pragma once

#include <winrt/base.h>
#include <winrt/Microsoft.UI.Xaml.Data.h>


namespace winrt::RedDotScanner::implementation
{

	template <class D>
	class NotifyPropertyChangedImplT
	{
	public:
		winrt::event_token PropertyChanged(winrt::Microsoft::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
		{
			return m_PropertyChanged.add(handler);
		}

		void PropertyChanged(winrt::event_token const& token) noexcept
		{
			m_PropertyChanged.remove(token);
		}

	protected:
		void RaisePropertyChanged(winrt::hstring const& propertyName)
		{
			m_PropertyChanged(*static_cast<D*>(this), Microsoft::UI::Xaml::Data::PropertyChangedEventArgs{propertyName});
		}

		winrt::event<Microsoft::UI::Xaml::Data::PropertyChangedEventHandler> m_PropertyChanged;
	};

}