/* File:    CPP_SIngleShotSample
* Author:  Ocean Optics Inc.        JK
*
* Date: 06/07/2008
*
* Note:
* This code is provided as-is for illustration only.  Use this code at your own risk.
* This program written to use the OmniDriver files and demonstrate the acquisition
* of a spectrum using the wrapper class.
*/

#include "stdafx.h"
#include "ArrayTypes.h" // located in %OMNIDRIVER_HOME%\include
#include "ContinuousStrobeImpl.h"
#include "Wrapper.h"
#include "JString.h"

int main()
{
	JSTRING apiVersion;
	int		buildNumber;
	CONTINUOUSSTROBEIMPL_T continuousStrobeImplHandle;
	JSTRING firmwareVersion;
	GPIO_T	gpioHandle;
	int		index;
	long long maximumDelay;
	long long minimumDelay;
	int		minimumAllowedIntegrationTime; // units: microseconds
	int		numberOfPixels;
	int		numberOfSpectrometersAttached; // actually attached and talking to us
	JSTRING serialNumber;
	int		spectrometerIndex; // 0-n, identifies which spectrometer we are interacting with
	JSTRING spectrometerName;
	DOUBLEARRAY_T spectrumArrayHandle;
	double* spectrumValues;
	DOUBLEARRAY_T wavelengthArrayHandle;
	double	wavelengthIntercept;
	double	wavelengthCoefficient1;
	double	wavelengthCoefficient2;
	double	wavelengthCoefficient3;
	double* wavelengthValues;
	WRAPPER_T	wrapperHandle;

	printf("\nSpectrumTest Demo 1.03\n");
	printf("language: C/C++\n");
	printf("OmniDriver interface type: \"C\"\n");
	printf("API demonstrated: Wrapper\n");

	wrapperHandle = Wrapper_Create();

	buildNumber = Wrapper_getBuildNumber(wrapperHandle);
	printf("build number = %d\n", buildNumber);

	apiVersion = JString_Create();
	Wrapper_getApiVersion(wrapperHandle, apiVersion);
	printf("api version = %s\n", JString_getASCII(apiVersion));
	JString_Destroy(apiVersion);

	// The following call will populate an internal array of spectrometer objects 
	numberOfSpectrometersAttached = Wrapper_openAllSpectrometers(wrapperHandle);
	printf("number of spectrometers found = %d\n", numberOfSpectrometersAttached);
	if (numberOfSpectrometersAttached == 0)
		return 0; // there are no attached spectrometers

				  // We will arbitrarily use the first attached spectrometer
	spectrometerIndex = 0;

	// Display some information about this spectrometer
	firmwareVersion = JString_Create();
	serialNumber = JString_Create();
	spectrometerName = JString_Create();
	Wrapper_getSerialNumber(wrapperHandle, spectrometerIndex, serialNumber);
	Wrapper_getName(wrapperHandle, spectrometerIndex, spectrometerName);
	Wrapper_getFirmwareVersion(wrapperHandle, spectrometerIndex, firmwareVersion);
	printf("spectrometer type: %s   serial number: %s  firmware version: %s\n",
		JString_getASCII(spectrometerName),
		JString_getASCII(serialNumber),
		JString_getASCII(firmwareVersion));
	JString_Destroy(firmwareVersion);
	JString_Destroy(serialNumber);
	JString_Destroy(spectrometerName);


	// Demonstrate how to obtain and display wavelength coefficients
	wavelengthIntercept = Wrapper_getWavelengthIntercept(wrapperHandle, spectrometerIndex);
	wavelengthCoefficient1 = Wrapper_getWavelengthFirst(wrapperHandle, spectrometerIndex);
	wavelengthCoefficient2 = Wrapper_getWavelengthSecond(wrapperHandle, spectrometerIndex);
	wavelengthCoefficient3 = Wrapper_getWavelengthThird(wrapperHandle, spectrometerIndex);
	printf("wavelength intercept = %1.4f  1st: %1.4f  2nd: %1.4f  3rd: %1.4f\n",
		wavelengthIntercept,
		wavelengthCoefficient1, wavelengthCoefficient2, wavelengthCoefficient3);

	// Demonstrate how to access the "GPIO" feature
	if (Wrapper_isFeatureSupportedGPIO(wrapperHandle, spectrometerIndex) == 1)
	{
		gpioHandle = (GPIO_T)GPIO_Create();
		Wrapper_getFeatureControllerGPIO(wrapperHandle, spectrometerIndex, gpioHandle);
		printf("GPIO feature is supported\n");
		// At this point, you can access the GPIO functions
		// ...
		GPIO_Destroy(gpioHandle);
	}
	else {
		printf("GPIO feature is not supported\n");
	}

	// Demonstrate how to access the "continuous strobe" feature
	// Note: to use this feature, you must have the following include statement
	// #include "ContinuousStrobeImpl.h"
	if (Wrapper_isFeatureSupportedContinuousStrobe(wrapperHandle, spectrometerIndex) == 1)
	{
		continuousStrobeImplHandle = (CONTINUOUSSTROBEIMPL_T)ContinuousStrobeImpl_Create();
		Wrapper_getFeatureControllerContinuousStrobe(wrapperHandle, spectrometerIndex, continuousStrobeImplHandle);
		printf("ContinuousStrobe feature is supported\n");
		minimumDelay = ContinuousStrobeImpl_getContinuousStrobeDelayMinimum(continuousStrobeImplHandle);
		maximumDelay = ContinuousStrobeImpl_getContinuousStrobeDelayMaximum(continuousStrobeImplHandle);
		printf("ContinuousStrobe: minimumDelay = %lld\n", minimumDelay);
		printf("ContinuousStrobe: maximumDelay = %lld\n", maximumDelay);
		ContinuousStrobeImpl_Destroy(continuousStrobeImplHandle);
	}
	else {
		printf("ContinuousStrobe feature is not supported\n");
	}

	// Set some acquisition parameters
	minimumAllowedIntegrationTime = Wrapper_getMinimumIntegrationTime(wrapperHandle, spectrometerIndex);
	Wrapper_setIntegrationTime(wrapperHandle, spectrometerIndex, minimumAllowedIntegrationTime);
	Wrapper_setBoxcarWidth(wrapperHandle, spectrometerIndex, 0);
	Wrapper_setScansToAverage(wrapperHandle, spectrometerIndex, 1);
	Wrapper_setCorrectForElectricalDark(wrapperHandle, spectrometerIndex, 0);

	// Aquire a spectrum
	printf("\nPress <enter> to get a spectrum from this spectrometer\n");
	getchar();

	spectrumArrayHandle = DoubleArray_Create();
	Wrapper_getSpectrum(wrapperHandle, spectrometerIndex, spectrumArrayHandle);
	spectrumValues = DoubleArray_getDoubleValues(spectrumArrayHandle);
	numberOfPixels = DoubleArray_getLength(spectrumArrayHandle);

	wavelengthArrayHandle = (DOUBLEARRAY_T)DoubleArray_Create();
	Wrapper_getWavelengths(wrapperHandle, spectrometerIndex, wavelengthArrayHandle);
	wavelengthValues = DoubleArray_getDoubleValues(wavelengthArrayHandle);

	// Loop to print the spectral data to the screen
	for (index = 0; index<numberOfPixels; index++)
	{
		printf("pixel %4d = %5.2f  wavelength(nm): %1.2f\n", index, spectrumValues[index], wavelengthValues[index]);
	}
	DoubleArray_Destroy(spectrumArrayHandle);
	DoubleArray_Destroy(wavelengthArrayHandle);

	// Clean up
	Wrapper_closeAllSpectrometers(wrapperHandle);

	Wrapper_Destroy(wrapperHandle);

	printf("\nPress <enter> to exit the app normally\n");
	getchar();
}