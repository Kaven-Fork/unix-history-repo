/*
 * extensions to ioctl_meteor.h for the bt848 cards
 */

/*
 * frequency sets
 */
#define CHNLSET_NABCST		1
#define CHNLSET_CABLEIRC	2
#define CHNLSET_CABLEHRC	3
#define CHNLSET_WEUROPE		4
#define CHNLSET_MIN		CHNLSET_NABCST
#define CHNLSET_MAX		CHNLSET_WEUROPE


/*
 * constants for various tuner registers
 */
#define BT848_HUEMIN		(-90)
#define BT848_HUEMAX		90
#define BT848_HUECENTER		0
#define BT848_HUERANGE		179.3
#define BT848_HUEREGMIN		(-128)
#define BT848_HUEREGMAX		127
#define BT848_HUESTEPS		256

#define BT848_BRIGHTMIN		(-50)
#define BT848_BRIGHTMAX		50
#define BT848_BRIGHTCENTER	0
#define BT848_BRIGHTRANGE	99.6
#define BT848_BRIGHTREGMIN	(-128)
#define BT848_BRIGHTREGMAX	127
#define BT848_BRIGHTSTEPS	256

#define BT848_CONTRASTMIN	0
#define BT848_CONTRASTMAX	237
#define BT848_CONTRASTCENTER	100
#define BT848_CONTRASTRANGE	236.57
#define BT848_CONTRASTREGMIN	0
#define BT848_CONTRASTREGMAX	511
#define BT848_CONTRASTSTEPS	512

#define BT848_CHROMAMIN		0
#define BT848_CHROMAMAX		284
#define BT848_CHROMACENTER	100
#define BT848_CHROMARANGE	283.89
#define BT848_CHROMAREGMIN	0
#define BT848_CHROMAREGMAX	511
#define BT848_CHROMASTEPS	512

#define BT848_SATUMIN		0
#define BT848_SATUMAX		202
#define BT848_SATUCENTER	100
#define BT848_SATURANGE		201.18
#define BT848_SATUREGMIN	0
#define BT848_SATUREGMAX	511
#define BT848_SATUSTEPS		512

#define BT848_SATVMIN		0
#define BT848_SATVMAX		284
#define BT848_SATVCENTER	100
#define BT848_SATVRANGE		283.89
#define BT848_SATVREGMIN	0
#define BT848_SATVREGMAX	511
#define BT848_SATVSTEPS		512


/*
 * audio stuff
 */
#define AUDIO_TUNER		0x00	/* command for the audio routine */
#define AUDIO_EXTERN		0x01	/* don't confuse them with bit */
#define AUDIO_INTERN		0x02	/* settings */
#define AUDIO_MUTE		0x80
#define AUDIO_UNMUTE		0x81


/*
 * EEProm stuff
 */
struct eeProm {
	u_char bytes[ 256 ];
};


/*
 * XXX: this is a hack, should be in ioctl_meteor.h
 * here to avoid touching that file for now...
 */
#define	TVTUNER_SETCHNL    _IOW('x', 32, unsigned int)	/* set channel */
#define	TVTUNER_GETCHNL    _IOR('x', 32, unsigned int)	/* get channel */
#define	TVTUNER_SETTYPE    _IOW('x', 33, unsigned int)	/* set tuner type */
#define	TVTUNER_GETTYPE    _IOR('x', 33, unsigned int)	/* get tuner type */
#define	TVTUNER_GETSTATUS  _IOR('x', 34, unsigned int)	/* get tuner status */
#define	TVTUNER_SETFREQ    _IOW('x', 35, unsigned int)	/* set frequency */
#define	TVTUNER_GETFREQ    _IOR('x', 36, unsigned int)	/* get frequency */


#define BT848_SHUE	_IOW('x', 37, int)		/* set hue */
#define BT848_GHUE	_IOR('x', 37, int)		/* get hue */
#define	BT848_SBRIG	_IOW('x', 38, int)		/* set brightness */
#define BT848_GBRIG	_IOR('x', 38, int)		/* get brightness */
#define	BT848_SCSAT	_IOW('x', 39, int)		/* set chroma sat */
#define BT848_GCSAT	_IOR('x', 39, int)		/* get UV saturation */
#define	BT848_SCONT	_IOW('x', 40, int)		/* set contrast */
#define	BT848_GCONT	_IOR('x', 40, int)		/* get contrast */
#define	BT848_SVSAT	_IOW('x', 41, int)		/* set chroma V sat */
#define BT848_GVSAT	_IOR('x', 41, int)		/* get V saturation */
#define	BT848_SUSAT	_IOW('x', 42, int)		/* set chroma U sat */
#define BT848_GUSAT	_IOR('x', 42, int)		/* get U saturation */

#define	BT848_SCBARS	_IOR('x', 43, int)		/* set colorbar */
#define	BT848_CCBARS	_IOR('x', 44, int)		/* clear colorbar */

#define	BT848_EEPROM	_IOR('x', 45, struct eeProm)

#define	BT848_SAUDIO	_IOW('x', 46, int)		/* set audio channel */
#define BT848_GAUDIO	_IOR('x', 47, int)		/* get audio channel */
#define	BT848_SBTSC	_IOW('x', 48, int)		/* set audio channel */

/*
 * XXX: more bad magic,
 *      we need to fix the METEORGINPUT to return something public
 *      duplicate them here for now...
 */
#define	METEOR_DEV0		0x00001000
#define	METEOR_DEV1		0x00002000
#define	METEOR_DEV2		0x00004000
