#include "include/config.h"
#include "include/fann.h"

FANN_EXTERNAL void FANN_API fann_sigmoid_real(float sum, fann_type* ret)
{
    double tmp;
    
    mexp(-2.0f * sum, &tmp);
    *ret = 1.0f/(1.0f + (float)tmp);
    return;
}

FANN_EXTERNAL void FANN_API fann_sigmoid_symmetric_real(float sum, fann_type* ret)
{
    double tmp;
    
    mexp(-2.0f * sum, &tmp);
    *ret = 2.0f/(1.0f + (float)tmp) - 1.0f;
    return;
}

FANN_EXTERNAL void FANN_API fann_gaussian_real(float sum, fann_type *ret) 
{
    double tmp;
    mexp(-sum * sum, &tmp);
    *ret = (float)tmp;
} 

FANN_EXTERNAL void FANN_API fann_gaussian_symmetric_real(float sum, fann_type* ret)
{
    double tmp;
    mexp(-sum * sum, &tmp);
    *ret = (tmp * 2.0f) - 1.0f;
}

FANN_EXTERNAL void FANN_API fann_activation_switch(unsigned int activation_function, fann_type value, fann_type* result)
{
    switch(activation_function)  
    {				 
    case FANN_LINEAR:  
	*result = (fann_type)value;  
        break;  
    case FANN_LINEAR_PIECE:  
	*result = (fann_type)((value < 0) ? 0 : (value > 1) ? 1 : value);  
        break;  
    case FANN_LINEAR_PIECE_SYMMETRIC:  
	*result = (fann_type)((value < -1) ? -1 : (value > 1) ? 1 : value);  
        break;  
    case FANN_SIGMOID:  
        fann_sigmoid_real(value, result);  
        break;  
    case FANN_SIGMOID_SYMMETRIC:  
	//*result = (fann_type)fann_sigmoid_symmetric_real(value);  
	fann_sigmoid_symmetric_real(value, result);
        break;  
    case FANN_SIGMOID_SYMMETRIC_STEPWISE:  
	*result = (fann_type)fann_stepwise(-2.64665293693542480469e+00, -1.47221934795379638672e+00, -5.49306154251098632812e-01, 5.49306154251098632812e-01, 1.47221934795379638672e+00, 2.64665293693542480469e+00, -9.90000009536743164062e-01, -8.99999976158142089844e-01, -5.00000000000000000000e-01, 5.00000000000000000000e-01, 8.99999976158142089844e-01, 9.90000009536743164062e-01, -1, 1, value);  
        break;  
    case FANN_SIGMOID_STEPWISE:  
	*result = (fann_type)fann_stepwise(-2.64665246009826660156e+00, -1.47221946716308593750e+00, -5.49306154251098632812e-01, 5.49306154251098632812e-01, 1.47221934795379638672e+00, 2.64665293693542480469e+00, 4.99999988824129104614e-03, 5.00000007450580596924e-02, 2.50000000000000000000e-01, 7.50000000000000000000e-01, 9.49999988079071044922e-01, 9.95000004768371582031e-01, 0, 1, value);  
        break;  
    case FANN_THRESHOLD:  
	*result = (fann_type)((value < 0) ? 0 : 1);  
        break;  
    case FANN_THRESHOLD_SYMMETRIC:  
	*result = (fann_type)((value < 0) ? -1 : 1);  
        break;  
    case FANN_GAUSSIAN:  
        //esult = (fann_type)fann_gaussian_real(value);  
	fann_gaussian_real(value, result);
        break;  
    case FANN_GAUSSIAN_SYMMETRIC:  
	//*result = (fann_type)fann_gaussian_symmetric_real(value);  
	fann_gaussian_symmetric_real(value, result);
        break;  
    case FANN_ELLIOT:  
	*result = (fann_type)fann_elliot_real(value);  
	break;  
    case FANN_ELLIOT_SYMMETRIC:  
	*result = (fann_type)fann_elliot_symmetric_real(value);  
        break;  
    case FANN_SIN_SYMMETRIC:  
	*result = (fann_type)fann_sin_symmetric_real(value);  
        break;  
    case FANN_COS_SYMMETRIC:  
	*result = (fann_type)fann_cos_symmetric_real(value);  
        break;  
    case FANN_SIN:  
	*result = (fann_type)fann_sin_real(value);  
	break;  
    case FANN_COS:  
	*result = (fann_type)fann_cos_real(value);  
	break;  
    case FANN_GAUSSIAN_STEPWISE:  
	*result = 0;  
        break;  
    }
}
