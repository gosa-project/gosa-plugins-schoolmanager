<?php

/*
 * This code is an addon for GOsa² (https://gosa.gonicus.de)
 * Copyright (C) 2022 Daniel Teichmann
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

namespace SchoolManager\Objects\Traits;

trait Gender
{
    /**
     * @var string $gender (M, F or D)
     */
    private $gender;

    /**
     * @param  string $gender Gender of this user.
     *                        For format see Utils->normalizeGenderString()
     * @return bool true if input was valid, false if invalid.
     */
    public function setGender(string $gender): bool
    {
        $returned_gender = $this->utils->normalizeGenderString($gender);

        if (empty($returned_gender)) {
            return false;
        } else {
            $this->gender = $returned_gender;
            return true;
        }
    }

    /**
     * @return string Gender in format 'F' => Female, 'M' => Male or 'D' => Diverse
     *                or **empty** if unknown.
     */
    public function getGender(): string
    {
        return is_null($this->gender) ? "" : $this->gender;
    }
}
