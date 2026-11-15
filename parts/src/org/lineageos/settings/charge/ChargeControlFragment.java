/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package org.lineageos.settings.charge;

import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;

import androidx.preference.ListPreference;
import androidx.preference.Preference;
import androidx.preference.SwitchPreferenceCompat;

import com.android.settingslib.widget.SettingsBasePreferenceFragment;

import org.lineageos.settings.R;

/**
 * Peridot charging dashboard. Charge limits and schedules remain owned by
 * Lineage Health; this fragment only links to that UI and controls the
 * dedicated kernel bypass ABI used by the QS tile.
 */
public class ChargeControlFragment extends SettingsBasePreferenceFragment
        implements Preference.OnPreferenceChangeListener {
    private static final String SYSTEM_CHARGE_CONTROL = "system_charge_control";
    private static final String BYPASS_CHARGING = "bypass_charging_pref";
    private static final String BYPASS_MODE = "bypass_mode_pref";
    private static final long POLL_INTERVAL_MS = 250L;
    private static final long POLL_TIMEOUT_MS = 8000L;

    private final Handler mHandler = new Handler(Looper.getMainLooper());
    private SwitchPreferenceCompat mBypassPreference;
    private ListPreference mModePreference;
    private long mPollDeadline;

    @Override
    public void onCreatePreferences(Bundle savedInstanceState, String rootKey) {
        addPreferencesFromResource(R.xml.charge_control_settings);

        Preference systemControl = findPreference(SYSTEM_CHARGE_CONTROL);
        systemControl.setOnPreferenceClickListener(preference -> {
            startActivity(new Intent(Intent.ACTION_POWER_USAGE_SUMMARY));
            return true;
        });

        mBypassPreference = findPreference(BYPASS_CHARGING);
        mModePreference = findPreference(BYPASS_MODE);
        mBypassPreference.setOnPreferenceChangeListener(this);
        mModePreference.setOnPreferenceChangeListener(this);
    }

    @Override
    public void onResume() {
        super.onResume();
        refreshState();
    }

    @Override
    public void onPause() {
        super.onPause();
        mHandler.removeCallbacksAndMessages(null);
    }

    @Override
    public boolean onPreferenceChange(Preference preference, Object newValue) {
        if (BYPASS_MODE.equals(preference.getKey())) {
            String mode = (String) newValue;
            if (!BypassChargingUtils.writeMode(mode)) {
                return false;
            }
            BypassChargingUtils.saveMode(requireContext(), mode);
            updateModeSummary(mode, false);
            return true;
        }

        if (BYPASS_CHARGING.equals(preference.getKey())) {
            boolean enabled = (Boolean) newValue;
            mBypassPreference.setEnabled(false);
            mBypassPreference.setSummary(R.string.bypass_charging_summary_pending);
            if (!BypassChargingUtils.requestState(enabled)) {
                refreshState();
                return false;
            }
            mPollDeadline = System.currentTimeMillis() + POLL_TIMEOUT_MS;
            mHandler.postDelayed(mPollRunnable, POLL_INTERVAL_MS);
            return true;
        }

        return false;
    }

    private final Runnable mPollRunnable = new Runnable() {
        @Override
        public void run() {
            BypassChargingUtils.State state = BypassChargingUtils.readState();
            applyState(state);
            if (BypassChargingUtils.isPending(state)
                    && System.currentTimeMillis() < mPollDeadline) {
                mHandler.postDelayed(this, POLL_INTERVAL_MS);
            }
        }
    };

    private void refreshState() {
        applyState(BypassChargingUtils.readState());
    }

    private void applyState(BypassChargingUtils.State state) {
        mBypassPreference.setChecked(BypassChargingUtils.isOn(state));
        switch (state) {
            case ON:
                mBypassPreference.setSummary(R.string.bypass_charging_summary_on);
                mBypassPreference.setEnabled(true);
                break;
            case OFF:
                mBypassPreference.setSummary(R.string.bypass_charging_summary_off);
                mBypassPreference.setEnabled(true);
                break;
            case PENDING_ON:
            case PENDING_OFF:
                mBypassPreference.setSummary(R.string.bypass_charging_summary_pending);
                mBypassPreference.setEnabled(false);
                break;
            case UNAVAILABLE:
            default:
                mBypassPreference.setSummary(R.string.bypass_charging_summary_unavailable);
                mBypassPreference.setEnabled(false);
                break;
        }
        refreshMode(state);
    }

    private void refreshMode(BypassChargingUtils.State state) {
        String mode = BypassChargingUtils.readMode();
        if (mode == null || state == BypassChargingUtils.State.UNAVAILABLE) {
            mModePreference.setEnabled(false);
            return;
        }
        mModePreference.setValue(mode);
        boolean locked = BypassChargingUtils.isOn(state)
                || BypassChargingUtils.isPending(state);
        mModePreference.setEnabled(!locked);
        updateModeSummary(mode, locked);
    }

    private void updateModeSummary(String mode, boolean locked) {
        int index = mModePreference.findIndexOfValue(mode);
        if (index < 0) {
            return;
        }
        CharSequence label = mModePreference.getEntries()[index];
        mModePreference.setSummary(locked
                ? getString(R.string.bypass_mode_locked, label) : label);
    }
}
