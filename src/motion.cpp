#include "motion.h"

#ifdef MOTION_ENABLED
LIS3DH g_motionsensor(I2C_MODE, 0x18);
uint8_t MotionHelper::GetMotionInterupts()
{
    uint8_t motionresult = 0;
    uint8_t motionint1data = 0, motionint2data = 0;
    g_motionsensor.readRegister(&motionint1data, LIS3DH_INT1_SRC);
    g_motionsensor.readRegister(&motionint2data, LIS3DH_INT2_SRC);
    if ((motionint1data & LIS3DHEnums::INT_SRC::IA) == LIS3DHEnums::INT_SRC::IA)
    {
        motionresult = motionresult | 0x01;
    }
    if ((motionint2data & LIS3DHEnums::INT_SRC::IA) == LIS3DHEnums::INT_SRC::IA)
    {
        motionresult = motionresult | 0x10;
    }
    return motionresult;
}

void MotionHelper::InitMotionSensor(uint8_t firstThreshold, uint8_t secondThreshold, uint8_t firstDuration, uint8_t secondDuration)
{

    status_t sensorBeginResult = g_motionsensor.begin(false);

    if (sensorBeginResult != 0)
    {
        SERIAL_LOG("Failed to start motion sensor: %d", sensorBeginResult);
        LedHelper::BlinkHalt();
    }

    // configurations for control registers
    uint8_t latch = 0;
    g_motionsensor.writeRegister(LIS3DH_CTRL_REG1, 0); // power it down.
    g_motionsensor.writeRegister(LIS3DH_CTRL_REG0, LIS3DHEnums::CTRL_REG0::PullUpDisconnected);
    delay(250);
    g_motionsensor.writeRegister(LIS3DH_CTRL_REG1, (LIS3DHEnums::CTRL_REG1::ORD1 | LIS3DHEnums::CTRL_REG1::LPen | LIS3DHEnums::CTRL_REG1::XYZen)); // 10Hz, Low Power = 3 μA.
    g_motionsensor.writeRegister(LIS3DH_CTRL_REG2, (LIS3DHEnums::CTRL_REG2::HP_IA1 | LIS3DHEnums::CTRL_REG2::HP_IA2));
    if (firstThreshold > 0)
    {
        g_motionsensor.writeRegister(LIS3DH_CTRL_REG3, LIS3DHEnums::CTRL_REG3::I1_IA1);
    }
    else
    {
        g_motionsensor.writeRegister(LIS3DH_CTRL_REG3, 0);
    }
    g_motionsensor.writeRegister(LIS3DH_CTRL_REG4, LIS3DHEnums::CTRL_REG4::FS_2G); // 16mg
    g_motionsensor.writeRegister(LIS3DH_CTRL_REG5, latch);
    if (secondThreshold > 0)
    {
        g_motionsensor.writeRegister(LIS3DH_CTRL_REG6, LIS3DHEnums::CTRL_REG6::I2_IA2);
    }
    else
    {
        g_motionsensor.writeRegister(LIS3DH_CTRL_REG6, 0);
    }
    g_motionsensor.writeRegister(LIS3DH_REFERENCE, 0);

    if (firstThreshold > 0)
    {
        g_motionsensor.writeRegister(LIS3DH_INT1_THS, firstThreshold); // Threshold is value * REG4, so for us * 16mg
        g_motionsensor.writeRegister(LIS3DH_INT1_DURATION, firstDuration);
        g_motionsensor.writeRegister(LIS3DH_INT1_CFG, (LIS3DHEnums::INT_CFG::YHIE | LIS3DHEnums::INT_CFG::XHIE | LIS3DHEnums::INT_CFG::ZHIE));
        latch = latch | LIS3DHEnums::CTRL_REG5::LIR_INT1;
    }
    if (secondThreshold > 0)
    {
        g_motionsensor.writeRegister(LIS3DH_INT2_THS, secondThreshold);
        g_motionsensor.writeRegister(LIS3DH_INT2_DURATION, secondDuration);
        g_motionsensor.writeRegister(LIS3DH_INT2_CFG, (LIS3DHEnums::INT_CFG::YHIE | LIS3DHEnums::INT_CFG::XHIE | LIS3DHEnums::INT_CFG::ZHIE));
        latch = latch | LIS3DHEnums::CTRL_REG5::LIR_INT2;
    }
    g_motionsensor.writeRegister(LIS3DH_CTRL_REG5, latch);

    uint8_t dummy = 0;
    g_motionsensor.readRegister(&dummy, LIS3DH_REFERENCE); // reset to current position on initialize.

    SERIAL_LOG("Motion sensor initialized");
}

#endif