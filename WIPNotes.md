# Notes

## What we want?
- Link sleeptime to the thresholds that we had.
- This would mean we have 3 sleeptimes
    - no threshold
    - 1st threshold
    - 2nd threshold
- The no threshold would be the default and the ONLY sleeptime if there is no motion sensor present.
- If a threshold is hit, then the next sleeptime should be the highest threshold. This means
    - No threshold - stay with default timeout (still configurable)
    - 1st threshold hit - use this 1st threshold sleeptime.
    - 2nd threshold hit - use this 2nd threshold sleeptime. We're assuming that first threshold is always 'less' then 2nd threshold, so:
    - 1st and 2nd threshold hit - use 2nd threshold sleeptime.
- If there is NO 1st threshold sleep time set (it is 0), then we use the default. 
- if there is NO 2nd threshold sleeptime set (it is 0), then we use the 1st threshold sleeptime. If that is not set either, then we use the default.
    This allows us to configure 1st and 2nd threshold but have one 'sleeptime' for both.
- For now, we only want to configure Sleeptime based on threshold. There's a load of other parameters that we could configure as well, but we're skipping them for now (like GPS timeout, etc).



## How?
- add sleeptimer variables and make sure we can set them via the configuration options
- In the doPeriodicUpdate, we check for threshold data. The logic to check for 1st and 2nd threshold should be there
  - if the sleeptimer is different and we have a threshold event, then we need to /restart/ the timer like we do when we receive configruation.
  - I think this logic should /always/ set the timer if the timer delay is different?
  - SoftwareTimer.h has a setPeriod. Can we use that or do we still need to restart?
  - Should we have 3 timers and just turn on/off the correct one?
- When do we stop the increased updates?
    - We just check the thresholds again and reset it. This would be easiest and probably most correct.
    - We can do more advanced things where we wait for an additional update. This would give us at least 2 updates when we get an increase.
        - Doing so would require us to keep track of what update we have. And it would require that 0/1/2 threshold has a increasingly shorter sleeptime (which is not garanteed).
        - Also, what do we do if we have 1st threshold and then 2nd threshold? Then we'd have to /increase/ and do that increased update twice. Now what if we have update 2 and then update 1. 
        - It all just feels impractical and difficult to manage correctly.
- When we change configuration, we reset the timers. We'll just default to the default time again. This is again not great, but it allows us to also 'reset' the timers when we need to.
- We considered having multiple timers and  just turning on/off the correct one. This would be easier code, but does require us to manage the state, in combination with the change of configuration, htis actually is quite ugly. So one timer that we 'reset' when new config is received is ok.

