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

trait Birthday
{
    /**
     * @var string $birthday in Format 'Y.m.d'
     */
    private $birthday;

    /**
     * @param  string $birthday Birthday in format 'Y.m.d'
     * @return bool true if input was valid, false if invalid.
     */
    public function setBirthday(string $birthday): bool
    {
        $date = date_parse_from_format("Y.m.d", $birthday);
        if ($date['error_count']) {
            return false;
        }

        if (checkdate($date['month'], $date['day'], $date['year'])) {
            $this->birthday = $date['year'] . '.' . $date['month'] . '.' . $date['day'];
            return true;
        } else {
            return false;
        }
    }

    /**
     * @return string Birthday of user. (In format Y.m.d)
     */
    public function getBirthday(): string
    {
        return is_null($this->birthday) ? "" : $this->birthday;
    }
}
