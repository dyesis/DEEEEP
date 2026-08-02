#pragma once

// These will improve the readability of the connection definition

#define getT(Idx) template get<Idx>()
#define connectT(Idx, target) template connect<Idx>(target)
#define getParameterT(Idx) template getParameter<Idx>()
#define setParameterT(Idx, value) template setParameter<Idx>(value)
#define setParameterWT(Idx, value) template setWrapParameter<Idx>(value)
using namespace scriptnode;
using namespace snex;
using namespace snex::Types;

namespace swoop_impl
{
// ==============================| Node & Parameter type declarations |==============================

DECLARE_PARAMETER_RANGE_SKEW(dry_wet_mixer_c0Range, 
                             -100., 
                             0., 
                             5.42227);

template <int NV>
using dry_wet_mixer_c0 = parameter::from0To1<core::gain<NV>, 
                                             0, 
                                             dry_wet_mixer_c0Range>;

template <int NV> using dry_wet_mixer_c1 = dry_wet_mixer_c0<NV>;

template <int NV>
using dry_wet_mixer_multimod = parameter::list<dry_wet_mixer_c0<NV>, dry_wet_mixer_c1<NV>>;

template <int NV>
using dry_wet_mixer_t = control::xfader<dry_wet_mixer_multimod<NV>, 
                                        faders::cosine_half>;

template <int NV>
using dry_path_t = container::chain<parameter::empty, 
                                    wrap::fix<2, dry_wet_mixer_t<NV>>, 
                                    core::gain<NV>>;
template <int NumVoices> struct snex_shaper
{
	SNEX_NODE(snex_shaper);
	// Implement the Waveshaper here...
	float gain = 0.0f;
	float getSample(float input)
	{
		return (1.0f + 1.f) *input / (1.4f + 0.6f * Math.abs(input));
		return input;
	}
	// These functions are the glue code that call the function above
	template <typename T> void process(T& data)
	{
		for(auto ch: data)
		{
			for(auto& s: data.toChannelData(ch))
			{
				s = getSample(s);
			}
		}
	}
	template <typename T> void processFrame(T& data)
	{
		for(auto& s: data)
			s = getSample(s);
	}
	void reset()
	{
	}
	void prepare(PrepareSpecs ps)
	{
	}
	void setExternalData(const ExternalData& d, int index)
	{
	}
	template <int P> void setParameter(double v)
	{
	}
};

template <int NV>
using snex_shaper_t = wrap::no_data<core::snex_shaper<snex_shaper<NV>>>;

template <int NV>
using wet_path_t = container::chain<parameter::empty, 
                                    wrap::fix<2, filters::svf_eq<NV>>, 
                                    snex_shaper_t<NV>, 
                                    core::gain<NV>, 
                                    fx::bitcrush<NV>, 
                                    math::tanh<NV>, 
                                    core::gain<NV>>;

namespace dry_wet1_t_parameters
{
}

template <int NV>
using dry_wet1_t = container::split<parameter::plain<swoop_impl::dry_wet_mixer_t<NV>, 0>, 
                                    wrap::fix<2, dry_path_t<NV>>, 
                                    wet_path_t<NV>>;

template <int NV>
using band1_t = container::chain<parameter::empty, 
                                 wrap::fix<2, jdsp::jlinkwitzriley>, 
                                 dry_wet1_t<NV>>;

template <int NV>
using band2_t = container::chain<parameter::empty, 
                                 wrap::fix<2, jdsp::jlinkwitzriley>, 
                                 math::tanh<NV>>;

namespace freq_split3_t_parameters
{
DECLARE_PARAMETER_RANGE_SKEW(Band1_InputRange, 
                             20., 
                             20000., 
                             0.229905);
DECLARE_PARAMETER_RANGE_SKEW(Band1_0Range, 
                             20., 
                             20000., 
                             0.229905);

using Band1_0 = parameter::from0To1<jdsp::jlinkwitzriley, 
                                    0, 
                                    Band1_0Range>;

using Band1_1 = Band1_0;

template <int NV>
using Band1 = parameter::chain<Band1_InputRange, 
                               Band1_0, 
                               Band1_1, 
                               parameter::plain<swoop_impl::dry_wet1_t<NV>, 0>>;

}

template <int NV>
using freq_split3_t = container::split<freq_split3_t_parameters::Band1<NV>, 
                                       wrap::fix<2, band1_t<NV>>, 
                                       band2_t<NV>>;

namespace swoop_t_parameters
{
DECLARE_PARAMETER_RANGE_SKEW(DryWetRange, 
                             20., 
                             20000., 
                             0.229905);

template <int NV>
using DryWet = parameter::from0To1<swoop_impl::freq_split3_t<NV>, 
                                   0, 
                                   DryWetRange>;

}

template <int NV>
using swoop_t_ = container::chain<swoop_t_parameters::DryWet<NV>, 
                                  wrap::fix<2, freq_split3_t<NV>>>;

// =================================| Root node initialiser class |=================================

template <int NV> struct instance: public swoop_impl::swoop_t_<NV>
{
	
	struct metadata
	{
		static const int NumTables = 0;
		static const int NumSliderPacks = 0;
		static const int NumAudioFiles = 0;
		static const int NumFilters = 0;
		static const int NumDisplayBuffers = 0;
		
		SNEX_METADATA_ID(swoop);
		SNEX_METADATA_NUM_CHANNELS(2);
		SNEX_METADATA_ENCODED_PARAMETERS(18)
		{
			0x005B, 0x0000, 0x4400, 0x7972, 0x6557, 0x0074, 0x0000, 0x0000, 
            0x0000, 0x3F80, 0x0000, 0x3F80, 0x0000, 0x3F80, 0x0000, 0x0000, 
            0x0000, 0x0000
		};
	};
	
	instance()
	{
		// Node References -------------------------------------------------------------------------
		
		auto& freq_split3 = this->getT(0);                                   // swoop_impl::freq_split3_t<NV>
		auto& band1 = this->getT(0).getT(0);                                 // swoop_impl::band1_t<NV>
		auto& lr1_1 = this->getT(0).getT(0).getT(0);                         // jdsp::jlinkwitzriley
		auto& dry_wet1 = this->getT(0).getT(0).getT(1);                      // swoop_impl::dry_wet1_t<NV>
		auto& dry_path = this->getT(0).getT(0).getT(1).getT(0);              // swoop_impl::dry_path_t<NV>
		auto& dry_wet_mixer = this->getT(0).getT(0).getT(1).getT(0).getT(0); // swoop_impl::dry_wet_mixer_t<NV>
		auto& dry_gain = this->getT(0).getT(0).getT(1).getT(0).getT(1);      // core::gain<NV>
		auto& wet_path = this->getT(0).getT(0).getT(1).getT(1);              // swoop_impl::wet_path_t<NV>
		auto& svf_eq = this->getT(0).getT(0).getT(1).getT(1).getT(0);        // filters::svf_eq<NV>
		auto& snex_shaper = this->getT(0).getT(0).getT(1).getT(1).getT(1);   // swoop_impl::snex_shaper_t<NV>
		auto& gain = this->getT(0).getT(0).getT(1).getT(1).getT(2);          // core::gain<NV>
		auto& bitcrush = this->getT(0).getT(0).getT(1).getT(1).getT(3);      // fx::bitcrush<NV>
		auto& tanh = this->getT(0).getT(0).getT(1).getT(1).getT(4);          // math::tanh<NV>
		auto& wet_gain = this->getT(0).getT(0).getT(1).getT(1).getT(5);      // core::gain<NV>
		auto& band2 = this->getT(0).getT(1);                                 // swoop_impl::band2_t<NV>
		auto& lr2_1 = this->getT(0).getT(1).getT(0);                         // jdsp::jlinkwitzriley
		auto& tanh1 = this->getT(0).getT(1).getT(1);                         // math::tanh<NV>
		
		// Parameter Connections -------------------------------------------------------------------
		
		dry_wet1.getParameterT(0).connectT(0, dry_wet_mixer); // DryWet -> dry_wet_mixer::Value
		auto& Band1_p = freq_split3.getParameterT(0);
		Band1_p.connectT(0, lr1_1);                      // Band1 -> lr1_1::Frequency
		Band1_p.connectT(1, lr2_1);                      // Band1 -> lr2_1::Frequency
		Band1_p.connectT(2, dry_wet1);                   // Band1 -> dry_wet1::DryWet
		this->getParameterT(0).connectT(0, freq_split3); // DryWet -> freq_split3::Band1
		
		// Modulation Connections ------------------------------------------------------------------
		
		auto& dry_wet_mixer_p = dry_wet_mixer.getWrappedObject().getParameter();
		dry_wet_mixer_p.getParameterT(0).connectT(0, dry_gain); // dry_wet_mixer -> dry_gain::Gain
		dry_wet_mixer_p.getParameterT(1).connectT(0, wet_gain); // dry_wet_mixer -> wet_gain::Gain
		
		// Default Values --------------------------------------------------------------------------
		
		; // freq_split3::Band1 is automated
		
		;                           // lr1_1::Frequency is automated
		lr1_1.setParameterT(1, 0.); // jdsp::jlinkwitzriley::Type
		
		; // dry_wet1::DryWet is automated
		
		; // dry_wet_mixer::Value is automated
		
		;                               // dry_gain::Gain is automated
		dry_gain.setParameterT(1, 20.); // core::gain::Smoothing
		dry_gain.setParameterT(2, 0.);  // core::gain::ResetValue
		
		svf_eq.setParameterT(0, 32.2669);  // filters::svf_eq::Frequency
		svf_eq.setParameterT(1, 0.624569); // filters::svf_eq::Q
		svf_eq.setParameterT(2, 6.77864);  // filters::svf_eq::Gain
		svf_eq.setParameterT(3, 0.);       // filters::svf_eq::Smoothing
		svf_eq.setParameterT(4, 4.);       // filters::svf_eq::Mode
		svf_eq.setParameterT(5, 1.);       // filters::svf_eq::Enabled
		
		gain.setParameterT(0, 0.);    // core::gain::Gain
		gain.setParameterT(1, 249.9); // core::gain::Smoothing
		gain.setParameterT(2, 0.);    // core::gain::ResetValue
		
		bitcrush.setParameterT(0, 13.9); // fx::bitcrush::BitDepth
		bitcrush.setParameterT(1, 1.);   // fx::bitcrush::Mode
		
		tanh.setParameterT(0, 1.); // math::tanh::Value
		
		;                               // wet_gain::Gain is automated
		wet_gain.setParameterT(1, 20.); // core::gain::Smoothing
		wet_gain.setParameterT(2, 0.);  // core::gain::ResetValue
		
		;                           // lr2_1::Frequency is automated
		lr2_1.setParameterT(1, 1.); // jdsp::jlinkwitzriley::Type
		
		tanh1.setParameterT(0, 1.); // math::tanh::Value
		
		this->setParameterT(0, 1.);
		this->setExternalData({}, -1);
	}
	~instance() override
	{
		// Cleanup external data references --------------------------------------------------------
		
		this->setExternalData({}, -1);
	}
	
	static constexpr bool isPolyphonic() { return NV > 1; };
	
	static constexpr bool hasTail() { return true; };
	
	static constexpr bool isSuspendedOnSilence() { return false; };
	
	void setExternalData(const ExternalData& b, int index)
	{
		// External Data Connections ---------------------------------------------------------------
		
		this->getT(0).getT(0).getT(1).getT(1).getT(1).setExternalData(b, index); // swoop_impl::snex_shaper_t<NV>
	}
};
}

#undef getT
#undef connectT
#undef setParameterT
#undef setParameterWT
#undef getParameterT
// ======================================| Public Definition |======================================

namespace project
{
// polyphonic template declaration

template <int NV>
using swoop = wrap::node<swoop_impl::instance<NV>>;
}


