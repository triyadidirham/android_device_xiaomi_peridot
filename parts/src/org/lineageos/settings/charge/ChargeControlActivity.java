/*
 * Copyright (C) 2026 The LineageOS Project
 *
 * SPDX-License-Identifier: Apache-2.0
 */
package org.lineageos.settings.charge;

import android.os.Bundle;

import com.android.settingslib.collapsingtoolbar.CollapsingToolbarBaseActivity;

public class ChargeControlActivity extends CollapsingToolbarBaseActivity {
    private static final String TAG_CHARGE_CONTROL = "charge_control";

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setTitle(org.lineageos.settings.R.string.charge_control_title);
        getSupportFragmentManager().beginTransaction().replace(
                com.android.settingslib.collapsingtoolbar.R.id.content_frame,
                new ChargeControlFragment(), TAG_CHARGE_CONTROL).commit();
    }
}
