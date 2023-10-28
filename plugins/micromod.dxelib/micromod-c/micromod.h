
/*
	Returns a string containing version information.
*/
const char *micromod_get_version( void );

/*
	Calculate the length in bytes of a module file given the 1084-byte header.
	Returns -1 if the data is not recognised as a module.
*/
long micromod_calculate_mod_file_len( signed char *module_header );

/*
	Set the player to play the specified module data.
	Returns -1 if the data is not recognised as a module.
	Returns -2 if the sampling rate is less than 8000hz.
*/
long micromod_initialise( signed char *data, long sampling_rate );

/*
	Obtains song and instrument names from the module.
	The song name is returned as instrument 0.
	The name is copied into the location pointed to by string,
	and is at most 23 characters long, including the trailing null.
*/
void micromod_get_string( long instrument, char *string );

/*
	Returns the total song duration in samples at the current sampling rate.
*/
long micromod_calculate_song_duration( void );

/*
	Jump directly to a specific pattern in the sequence.
*/
void micromod_set_position( long pos );

/*
	Mute the specified channel.
	If channel is negative, un-mute all channels.
	Returns the number of channels.
*/
long micromod_mute_channel( long channel );

/*
	Set the playback gain.
	For 4-channel modules, a value of 64 can be used without distortion.
	For 8-channel modules, a value of 32 or less is recommended.
*/
void micromod_set_gain( long value );

/*
	Calculate the specified number of stereo samples of audio.
	Output buffer must be zeroed.
*/
void micromod_get_audio( short *output_buffer, long count );
