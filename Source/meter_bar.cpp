/* ----------------------------------------------------------------------------

   K-Meter
   =======
   Implementation of a K-System meter according to Bob Katz' specifications

   Copyright (c) 2010-2015 Martin Zuther (http://www.mzuther.de/)

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Thank you for using free software!

---------------------------------------------------------------------------- */

#include "meter_bar.h"

MeterBar::MeterBar(const String &componentName, int nCrestFactor, bool bExpanded, bool bHorizontal, bool bDisplayPeakMeter, int nSegmentHeight)
{
    setName(componentName);
    isExpanded = bExpanded;
    bHorizontalMeter = bHorizontal;
    displayPeakMeter = bDisplayPeakMeter;

    // this component does not have any transparent areas (increases
    // performance on redrawing)
    setOpaque(true);

    // to prevent the inherent round-off errors of float subtraction,
    // crest factor and limits are stored as integers representing
    // 0.1 dB steps
    if (nCrestFactor == 0)
    {
        nMeterCrestFactor = 0;

        nLimitTopBars = nMeterCrestFactor - 20;
        nLimitRedBars = -90;
        nLimitAmberBars = -180;
        nLimitGreenBars_1 = -400;
        nLimitGreenBars_2 = nLimitGreenBars_1;
    }
    else if (nCrestFactor == 12)
    {
        nMeterCrestFactor = +120;

        nLimitTopBars = nMeterCrestFactor - 20;
        nLimitRedBars = +40;
        nLimitAmberBars = 0;
        nLimitGreenBars_1 = -300;
        nLimitGreenBars_2 = nLimitGreenBars_1;
    }
    else if (nCrestFactor == 14)
    {
        nMeterCrestFactor = +140;

        nLimitTopBars = nMeterCrestFactor - 20;
        nLimitRedBars = +40;
        nLimitAmberBars = 0;
        nLimitGreenBars_1 = -300;
        nLimitGreenBars_2 = nLimitGreenBars_1;
    }
    else // K-20
    {
        nMeterCrestFactor = +200;

        nLimitTopBars = nMeterCrestFactor - 20;
        nLimitRedBars = +40;
        nLimitAmberBars = 0;
        nLimitGreenBars_1 = -240;
        nLimitGreenBars_2 = -300;
    }

    if (isExpanded)
    {
        nNumberOfBars = 134;
    }
    else
    {
        if (nCrestFactor == 0)
        {
            nNumberOfBars = 47;
        }
        else if (nCrestFactor == 12)
        {
            nNumberOfBars = 48;
        }
        else if (nCrestFactor == 14)
        {
            nNumberOfBars = 50;
        }
        else // K-20
        {
            nNumberOfBars = 51;
        }
    }

    nMainSegmentHeight = nSegmentHeight;

    fPeakLevel = 0.0f;
    fAverageLevel = 0.0f;

    fPeakLevelPeak = 0.0f;
    fAverageLevelPeak = 0.0f;

    int nThreshold = 0; // bar threshold (in 0.1 dB)

    if (isExpanded && (nMeterCrestFactor > 80))
    {
        nThreshold = +80 - nMeterCrestFactor; // zoom into important region
    }

    int nKmeterLevel = nThreshold + nMeterCrestFactor; // bar K-Meter level (in 0.1 dB)
    int nRange = 0; // bar level range (in 0.1 dB)
    int nColor = 0;

    MeterArray = new MeterSegment*[nNumberOfBars];

    for (int n = 0; n < nNumberOfBars; n++)
    {
        if (isExpanded)
        {
            nRange = 1;
        }
        else
        {
            if (nKmeterLevel > nLimitTopBars)
            {
                nRange = 5;
            }
            else if (nKmeterLevel > nLimitGreenBars_1)
            {
                nRange = 10;
            }
            else if (nKmeterLevel > nLimitGreenBars_2)
            {
                nRange = 60;
            }
            else
            {
                nRange = 100;
            }
        }

        if (nCrestFactor == 0)
        {
            if (nKmeterLevel <= -280)
            {
                nColor = 0;
            }
            else if (nKmeterLevel <= -220)
            {
                nColor = 1;
            }
            else if ((nKmeterLevel > -160) && (nKmeterLevel <= -100))
            {
                nColor = 2;
            }
            else if (nKmeterLevel > nLimitRedBars)
            {
                nColor = 0;
            }
            else if (nKmeterLevel > nLimitAmberBars)
            {
                nColor = 1;
            }
            else
            {
                nColor = 2;
            }
        }
        else
        {
            if (nKmeterLevel > nLimitRedBars)
            {
                nColor = 0;
            }
            else if (nKmeterLevel > nLimitAmberBars)
            {
                nColor = 1;
            }
            else
            {
                nColor = 2;
            }
        }

        nThreshold -= nRange;
        nKmeterLevel -= nRange;
        MeterArray[n] = new MeterSegment("MeterSegment #" + String(n) + " (" + componentName + ")", nThreshold * 0.1f, nRange * 0.1f, displayPeakMeter, nColor);

        addAndMakeVisible(MeterArray[n]);
    }
}


MeterBar::~MeterBar()
{
    for (int n = 0; n < nNumberOfBars; n++)
    {
        removeChildComponent(MeterArray[n]);
        delete MeterArray[n];
        MeterArray[n] = nullptr;
    }

    delete [] MeterArray;
    MeterArray = nullptr;

    deleteAllChildren();
}


void MeterBar::paint(Graphics &g)
{
    g.fillAll(Colours::black);
}


void MeterBar::resized()
{
    int x = 0;
    int y = 0;
    int nWidth;
    int nHeight;
    int nSegmentHeight = nMainSegmentHeight;

    if (bHorizontalMeter)
    {
        nWidth = 134 * nMainSegmentHeight + 1;
        nHeight = getHeight();
    }
    else
    {
        nWidth = getWidth();;
        nHeight = 134 * nMainSegmentHeight + 1;
    }

    int nKmeterLevel = nMeterCrestFactor; // bar K-Meter level (in 0.1 dB)
    int nRange = 0; // bar level range (in 0.1 dB)

    for (int n = 0; n < nNumberOfBars; n++)
    {
        if (isExpanded)
        {
            nRange = 1;
        }
        else
        {
            if (nKmeterLevel > nLimitTopBars)
            {
                nRange = 5;
            }
            else if (nKmeterLevel > nLimitGreenBars_1)
            {
                nRange = 10;
            }
            else if (nKmeterLevel > nLimitGreenBars_2)
            {
                nRange = 60;
            }
            else
            {
                nRange = 100;
            }
        }

        if (isExpanded)
        {
            nSegmentHeight = nMainSegmentHeight;
        }
        else if (nKmeterLevel > nLimitTopBars)
        {
            nSegmentHeight = nMainSegmentHeight;
        }
        else if (nKmeterLevel > nLimitGreenBars_1)
        {
            nSegmentHeight = 2 * nMainSegmentHeight;
        }
        else if (nKmeterLevel > nLimitGreenBars_2)
        {
            nSegmentHeight = 6 * nMainSegmentHeight;
        }
        else if (n == nNumberOfBars - 1)
        {
            if (nMeterCrestFactor == 0)
            {
                nSegmentHeight = 10 * nMainSegmentHeight;
            }
            else if (nMeterCrestFactor == +120)
            {
                nSegmentHeight = 14 * nMainSegmentHeight;
            }
            else if (nMeterCrestFactor == +140)
            {
                nSegmentHeight = 13 * nMainSegmentHeight;
            }
            else // K-20
            {
                nSegmentHeight = 10 * nMainSegmentHeight;
            }
        }
        else
        {
            if (nMeterCrestFactor == 0)
            {
                nSegmentHeight = 11 * nMainSegmentHeight;
            }
            else if (nMeterCrestFactor == +120)
            {
                nSegmentHeight = 12 * nMainSegmentHeight;
            }
            else if (nMeterCrestFactor == +140)
            {
                nSegmentHeight = 11 * nMainSegmentHeight;
            }
            else // K-20
            {
                nSegmentHeight = 10 * nMainSegmentHeight;
            }
        }

        if (bHorizontalMeter)
        {
            MeterArray[n]->setBounds(nWidth - x - (nSegmentHeight + 1), y, nSegmentHeight + 1, nHeight);
            x += nSegmentHeight;
        }
        else
        {
            MeterArray[n]->setBounds(x, y, nWidth, nSegmentHeight + 1);
            y += nSegmentHeight;
        }

        nKmeterLevel -= nRange;
    }
}


void MeterBar::setLevels(float peakLevel, float averageLevel, float peakLevelPeak, float averageLevelPeak)
{
    if ((peakLevel != fPeakLevel) || (averageLevel != fAverageLevel) || (peakLevelPeak != fPeakLevelPeak) || (averageLevelPeak != fAverageLevelPeak))
    {
        fPeakLevel = peakLevel;
        fAverageLevel = averageLevel;

        fPeakLevelPeak = peakLevelPeak;
        fAverageLevelPeak = averageLevelPeak;

        for (int n = 0; n < nNumberOfBars; n++)
        {
            MeterArray[n]->setLevels(fPeakLevel, fAverageLevel, fPeakLevelPeak, fAverageLevelPeak);
        }
    }
}


// Local Variables:
// ispell-local-dictionary: "british"
// End:
