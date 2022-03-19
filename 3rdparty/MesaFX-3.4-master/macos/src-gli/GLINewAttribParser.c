#define GLI_MINIMUM_POLICY_BIT		1
#define GLI_MAXIMUM_POLICY_BIT		2
#define GLI_CLOSEST_POLICY_BIT		3
static GLboolean parseGLIAttribList(
				const GLint		*attribs,
				GLIPixelFormat 		*outPixFmt,
				GLbitfield		*outPolicy
				/* GLI_CLOSEST,GLI_MAXIMUM,GLI_MINIMUM */)
{
	GLint 			*parseList;
	GLIPixelFormat	minPixFmt;
	
	minPixFmt.rgba_mode = GL_FALSE;
	minPixFmt.double_buffer = GL_FALSE;
	minPixFmt.stereo_mode = GL_FALSE;
	minPixFmt.level = 0;
	
	minPixFmt.pixel_size = 0;
	minPixFmt.buffer_size = 0;
	minPixFmt.red_size = 0;
	minPixFmt.green_size = 0;
	minPixFmt.blue_size = 0;
	minPixFmt.alpha_size = 0;
	minPixFmt.aux_buffers = 0;
	minPixFmt.depth_size = 0;
	minPixFmt.stencil_size = 0;
	minPixFmt.accum_red_size = 0;
	minPixFmt.accum_green_size = 0;
	minPixFmt.accum_blue_size = 0;
	minPixFmt.accum_alpha_size = 0;
	
	minPixFmt.compliance = GLI_NONE;
	
	/* Set the default values */
	minPixFmt.next_pixel_format = NULL;
	minPixFmt.renderer_id = 0;
	minPixFmt.num_devices = 0;
	
	minPixFmt.os_support = GLI_NONE;
	
	minPixFmt.msb_performance = GLI_NONE;
	minPixFmt.lsb_performance = GLI_NONE;
	
	minPixFmt.compliant = GL_FALSE;
	minPixFmt.accelerated = GL_FALSE;
	minPixFmt.robust = GL_FALSE;
	minPixFmt.no_recovery = GL_FALSE;
	minPixFmt.mp_safe = GL_FALSE;

	*outPolicy = 0;
	
	/*
  	 * Fill out the pixel-format from the attribs
  	 */
  	parseList = (GLint*)attribs; 
  	
  	while(*parseList != GLI_NONE)
  	{
  		GLint parseCmd = (*parseList);
  		GLint currentCmd;
    	parseList++;
    	switch(parseCmd) 
    	{
      		case GLI_ALL_RENDERERS:	
      			minPixFmt.compliance = GLI_NONE;
        		break;
      		case GLI_BUFFER_SIZE:
        		currentCmd = *parseList++;
        		minPixFmt.buffer_size = currentCmd;
        		break;
      		case GLI_LEVEL:
      			currentCmd = *parseList++;
      			minPixFmt.level = currentCmd; 
        		break; 
      		case GLI_RGBA:
        		minPixFmt.rgba_mode = GL_TRUE;
        		break;
      		case GLI_DOUBLEBUFFER:
        		minPixFmt.double_buffer = GL_TRUE;
        		break;
        		
      		case GLI_STEREO:
        		minPixFmt.stereo_mode = GL_TRUE;
        		break;
        		
      		case GLI_AUX_BUFFERS:
      			currentCmd = *parseList++;
        		minPixFmt.aux_buffers = currentCmd;
       		 	break;
       		 	
      		case GLI_RED_SIZE:
      			currentCmd = *parseList++;
      			minPixFmt.red_size = currentCmd;
      			break;
      			
      		case GLI_GREEN_SIZE:
      			currentCmd = *parseList++;
      			minPixFmt.green_size = currentCmd;      
      			break;
      			
      		case GLI_BLUE_SIZE:
      			currentCmd = *parseList++;
      			minPixFmt.blue_size = currentCmd;        
      			break;
      			
      		case GLI_ALPHA_SIZE:
      			currentCmd = *parseList++;
      			minPixFmt.alpha_size = currentCmd;    
        		break;
      
      		case GLI_DEPTH_SIZE:
        		currentCmd = *parseList++;
				minPixFmt.depth_size = currentCmd;
        		break;
        		
      		case GLI_STENCIL_SIZE:
        		currentCmd = *parseList++;
        		minPixFmt.stencil_size = currentCmd;
        		break;
        		
      		case GLI_ACCUM_RED_SIZE:
        		currentCmd = *parseList++;
        		minPixFmt.accum_red_size = currentCmd;
        		break;
        		
      		case GLI_ACCUM_GREEN_SIZE:
        		currentCmd = *parseList++;
        		minPixFmt.accum_green_size = currentCmd;
      			break;
      			
      		case GLI_ACCUM_BLUE_SIZE:
        		currentCmd = *parseList++;
        		minPixFmt.accum_blue_size = currentCmd;
        		break;
        		
      		case GLI_ACCUM_ALPHA_SIZE:
      			currentCmd = *parseList++;
        		minPixFmt.accum_alpha_size = currentCmd;
        		break;
        	/* Exrended attributes: */
        	case GLI_OFFSCREEN:
        		minPixFmt.os_support |= GLI_OFFSCREEN_RENDERING_BIT;
        		break;
        	case GLI_FULLSCREEN: 
        		minPixFmt.os_support |= GLI_FULLSCREEN_RENDERING_BIT;
        		break;
        	case GLI_PIXEL_SIZE:
        		currentCmd = *parseList++;
        		minPixFmt.pixel_size = currentCmd;
        		break;
        	/* 2.0 Attributes */
        	case GLI_ACCELERATED:
        		minPixFmt.accelerated = GL_TRUE;
        		break;
        	case GLI_MP_SAFE:
        		minPixFmt.mp_safe  = GL_TRUE;
        		break;
        	case GLI_ROBUST:
        		minPixFmt.robust =GL_TRUE;
        		break;
        	case GLI_NO_RECOVERY:
        		minPixFmt.no_recovery = GL_TRUE;
        		break;
        	/* policy modes */
        	case GLI_MINIMUM_POLICY:
        		*outPolicy = GLI_MINIMUM_POLICY;
        		break;
        	case GLI_MAXIMUM_POLICY:
        		*outPolicy = GLI_MAXIMUM_POLICY;
        		break;
        	case GLI_CLOSEST_POLICY:
        		*outPolicy = GLI_CLOSEST_POLICY;
        		break;
       		default:
        		return GL_FALSE;
    	}
  	}
  	*outPixFmt = minPixFmt;
  	
  	return GL_TRUE;
}